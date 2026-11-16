// Jarvis-son V1 - Orquestador por turnos (Termux / Android ARM64)
// LISTO -> GRABANDO -> TRANSCRIBIENDO -> GENERANDO -> HABLANDO -> LISTO (o ERROR)
// Seguridad: NUNCA se usa system()/popen(). Cada programa se lanza con execvp y
// argumentos separados, asi el texto hablado no puede convertirse en una orden.

#include <fcntl.h>
#include <poll.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
using Reloj = std::chrono::steady_clock;

// ---------------- CONFIGURACION (ajusta aqui) ----------------
struct Config {
    std::string home, whisperBin, whisperModelo, llamaBin, llamaModelo, ttsScript, logDir;
    int segGrabacion = 6;  // el plan sugiere clips de 5-8 s
    int ctx = 1024;        // contexto inicial del plan
    int maxTokens = 96;    // salida maxima del plan
    int hilos = 4;
    int toSTT = 90, toSLM = 180, toTTS = 90, toCorto = 20;  // limites de tiempo (s)
    std::vector<std::string> llamaExtra = {};
    std::string sistema =
        "Eres Jarvis-son, un asistente de voz. Responde siempre en español latino "
        "neutro, en máximo dos frases cortas, sin listas ni símbolos.";
};

static Config crearConfig() {
    Config c;
    const char* h = std::getenv("HOME");
    c.home = h ? h : ".";
    c.whisperBin = c.home + "/whisper.cpp/build/bin/whisper-cli";
    c.whisperModelo = c.home + "/whisper.cpp/models/ggml-base.bin";
    c.llamaBin = c.home + "/llama.cpp/build/bin/llama-completion";
    c.llamaModelo = c.home + "/llama.cpp/models/qwen2.5-1.5b-instruct-q4_0.gguf";
    c.ttsScript = c.home + "/jarvis-son/scripts/tts.py";
    c.logDir = c.home + "/jarvis-son/logs";
    return c;
}

// ---------------- ESTADOS ----------------
enum class Estado { LISTO, GRABANDO, TRANSCRIBIENDO, GENERANDO, HABLANDO, ERROR };

static const char* nombre(Estado e) {
    switch (e) {
        case Estado::LISTO: return "LISTO";
        case Estado::GRABANDO: return "GRABANDO";
        case Estado::TRANSCRIBIENDO: return "TRANSCRIBIENDO";
        case Estado::GENERANDO: return "GENERANDO";
        case Estado::HABLANDO: return "HABLANDO";
        case Estado::ERROR: return "ERROR";
    }
    return "?";
}
static void mostrar(Estado e) { std::cout << "[" << nombre(e) << "]" << std::endl; }

// ---------------- UTILIDADES ----------------
static volatile sig_atomic_t g_salir = 0;
static void alInterrumpir(int) { g_salir = 1; }

static double seg(Reloj::time_point a, Reloj::time_point b) {
    return std::chrono::duration<double>(b - a).count();
}

static std::string recortar(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static std::string colapsarEspacios(const std::string& s) {
    std::string r;
    bool esp = false;
    for (char c : s) {
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
            esp = true;
        } else {
            if (esp && !r.empty()) r += ' ';
            esp = false;
            r += c;
        }
    }
    return r;
}

static std::string colaArchivo(const std::string& ruta, size_t n) {
    std::ifstream f(ruta, std::ios::binary);
    if (!f) return "";
    std::string t((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (t.size() > n) t = t.substr(t.size() - n);
    return recortar(t);
}

static long tamano(const std::string& ruta) {
    std::error_code ec;
    auto s = fs::file_size(ruta, ec);
    return ec ? -1 : static_cast<long>(s);
}

static bool enPath(const std::string& prog) {
    const char* p = std::getenv("PATH");
    if (!p) return false;
    std::string path = p;
    size_t ini = 0;
    while (ini <= path.size()) {
        size_t fin = path.find(':', ini);
        if (fin == std::string::npos) fin = path.size();
        std::string dir = path.substr(ini, fin - ini);
        if (!dir.empty() && access((dir + "/" + prog).c_str(), X_OK) == 0) return true;
        ini = fin + 1;
    }
    return false;
}

// ---------------- LANZADOR DE PROCESOS ----------------
struct Resultado {
    int codigo = -1;  // codigo de salida; -1 no arranco, -2 tiempo agotado, -3 por señal
    std::string salida;   // stdout
    std::string errores;  // ultimas lineas de stderr
    double segundos = 0;
    long memKB = 0;  // memoria pico del proceso
};

static Resultado ejecutar(const std::vector<std::string>& args, int limiteSeg,
                          const std::string& archivoErr) {
    Resultado r;
    int tubo[2];
    if (pipe(tubo) != 0) return r;
    auto t0 = Reloj::now();
    pid_t pid = fork();
    if (pid < 0) {
        close(tubo[0]);
        close(tubo[1]);
        return r;
    }
    if (pid == 0) {  // proceso hijo
        dup2(tubo[1], STDOUT_FILENO);
        close(tubo[0]);
        close(tubo[1]);
        int nulo = open("/dev/null", O_RDONLY);
        if (nulo >= 0) {
            dup2(nulo, STDIN_FILENO);
            if (nulo > 2) close(nulo);
        }
        int ef = open(archivoErr.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (ef >= 0) {
            dup2(ef, STDERR_FILENO);
            if (ef > 2) close(ef);
        }
        std::vector<char*> av;
        for (const auto& a : args) av.push_back(const_cast<char*>(a.c_str()));
        av.push_back(nullptr);
        execvp(av[0], av.data());
        _exit(127);
    }
    close(tubo[1]);
    fcntl(tubo[0], F_SETFL, O_NONBLOCK);

    char buf[4096];
    bool eof = false, agotado = false;
    int st = 0;
    struct rusage uso {};
    auto vaciar = [&]() {
        ssize_t n;
        while ((n = read(tubo[0], buf, sizeof buf)) > 0) r.salida.append(buf, static_cast<size_t>(n));
        if (n == 0) eof = true;
    };
    while (true) {
        if (!eof) {
            struct pollfd p = {tubo[0], POLLIN, 0};
            poll(&p, 1, 100);
            vaciar();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        pid_t w = wait4(pid, &st, WNOHANG, &uso);
        if (w == pid) {
            vaciar();
            break;
        }
        if (w < 0 && errno != EINTR) break;
        if (seg(t0, Reloj::now()) > limiteSeg) {
            kill(pid, SIGKILL);
            wait4(pid, &st, 0, &uso);
            agotado = true;
            break;
        }
    }
    close(tubo[0]);
    r.segundos = seg(t0, Reloj::now());
    r.memKB = uso.ru_maxrss;
    r.codigo = agotado ? -2 : (WIFEXITED(st) ? WEXITSTATUS(st) : -3);
    r.errores = colaArchivo(archivoErr, 500);
    return r;
}

// --- LIMPIEZA DE TEXTO ---
static std::string quitarCorchetes(const std::string& s) {
    std::string r;
    int prof = 0;
    for (char c : s) {
        if (c == '[' || c == '(') prof++;
        else if ((c == ']' || c == ')') && prof > 0) prof--;
        else if (prof == 0) r += c;
    }
    return r;
}

static std::string sanitizarEntrada(std::string s) {
    s = quitarCorchetes(s);
    for (const char* t : {"<|", "|>"}) {
        size_t p;
        while ((p = s.find(t)) != std::string::npos) s.replace(p, 2, " ");
    }
    s = colapsarEspacios(s);
    if (s.size() > 300) {
        size_t c = 300;
        while (c > 0 && (static_cast<unsigned char>(s[c]) & 0xC0) == 0x80) c--;
        s.resize(c);
    }
    return s;
}

static std::string limpiarRespuesta(std::string s) {
    for (const char* t : {"<|im_end|>", "<|im_start|>", "<|endoftext|>", "[end of text]"}) {
        size_t p;
        while ((p = s.find(t)) != std::string::npos) s.erase(p, std::strlen(t));
    }
    s = recortar(s);
    size_t u = s.find_last_of(".!?");
    if (u != std::string::npos && u >= 15) s.resize(u + 1);
    return s;
}

static int contarPalabras(const std::string& s) {
    int n = 0;
    bool dentro = false;
    for (char c : s) {
        bool sp = (c == ' ');
        if (!sp && !dentro) n++;
        dentro = !sp;
    }
    return n;
}

static double duracionWav(const std::string& ruta) {
    std::ifstream f(ruta, std::ios::binary);
    unsigned char h[44];
    if (!f.read(reinterpret_cast<char*>(h), 44)) return 0;
    unsigned bps = static_cast<unsigned>(h[28]) | (static_cast<unsigned>(h[29]) << 8) |
                   (static_cast<unsigned>(h[30]) << 16) | (static_cast<unsigned>(h[31]) << 24);
    long total = tamano(ruta);
    return (bps && total > 44) ? static_cast<double>(total - 44) / bps : 0;
}

// --- METRICAS ---
struct Metricas {
    int turno = 0;
    double grab = 0, conv = 0, stt = 0, slm = 0, tts = 0, reprod = 0, total = 0;
    long memStt = 0, memSlm = 0, memTts = 0;
    int palEntrada = 0, palRespuesta = 0;
    std::string resultado = "ok";
};

static void guardarMetricas(const Config& c, const Metricas& m) {
    std::error_code ec;
    fs::create_directories(c.logDir, ec);
    std::string ruta = c.logDir + "/metricas.csv";
    bool nuevo = !fs::exists(ruta);
    std::ofstream f(ruta, std::ios::app);
    if (!f) return;
    if (nuevo)
        f << "turno,fecha,t_grab_s,t_conv_s,t_stt_s,t_slm_s,t_tts_s,t_reprod_s,"
             "t_hasta_voz_s,t_total_s,mem_stt_kb,mem_slm_kb,mem_tts_kb,"
             "palabras_entrada,palabras_respuesta,resultado\n";
    char fecha[32];
    std::time_t ahora = std::time(nullptr);
    std::strftime(fecha, sizeof fecha, "%Y-%m-%d %H:%M:%S", std::localtime(&ahora));
    f << std::fixed << std::setprecision(2) << m.turno << ',' << fecha << ',' << m.grab << ','
      << m.conv << ',' << m.stt << ',' << m.slm << ',' << m.tts << ',' << m.reprod << ','
      << (m.conv + m.stt + m.slm + m.tts) << ',' << m.total << ',' << m.memStt << ','
      << m.memSlm << ',' << m.memTts << ',' << m.palEntrada << ',' << m.palRespuesta << ','
      << m.resultado << '\n';
}

// --- ERROR VISIBLE ---
static bool fallo(Metricas& m, Estado e, const std::string& que, const Resultado& r) {
    m.resultado = std::string("error_") + nombre(e);
    mostrar(Estado::ERROR);
    std::cerr << "  Etapa: " << nombre(e) << "\n  Motivo: " << que << "\n";
    if (r.codigo == -2) std::cerr << "  Salida: tiempo agotado (se detuvo el programa)\n";
    else if (r.codigo == 127) std::cerr << "  Salida: 127 (programa no encontrado)\n";
    else if (r.codigo >= 0 || r.codigo == -3) std::cerr << "  Codigo de salida: " << r.codigo << "\n";
    if (!r.errores.empty()) std::cerr << "  Mensajes del programa:\n" << r.errores << "\n";
    std::cerr << "  Siguiente paso: ejecuta ese comando a mano y revisa el error completo.\n";
    return false;
}

// --- UN TURNO COMPLETO ---
static bool turno(const Config& c, Metricas& m, const std::string& dir) {
    const std::string m4a = dir + "/entrada.m4a", wav = dir + "/entrada.wav";
    const std::string resp = dir + "/respuesta.wav", err = dir + "/stderr.txt";
    Resultado r;

    // 1) GRABANDO
    mostrar(Estado::GRABANDO);
    auto t = Reloj::now();
    ejecutar({"termux-microphone-record", "-q"}, c.toCorto, err);
    r = ejecutar({"termux-microphone-record", "-f", m4a, "-l", std::to_string(c.segGrabacion)},
                 c.toCorto, err);
    if (r.codigo != 0) return fallo(m, Estado::GRABANDO, "no se pudo iniciar el microfono", r);
    std::cout << "  Habla ahora (" << c.segGrabacion << " s)..." << std::endl;
    sleep(static_cast<unsigned>(c.segGrabacion + 1));
    ejecutar({"termux-microphone-record", "-q"}, c.toCorto, err);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    m.grab = seg(t, Reloj::now());
    if (tamano(m4a) < 1000)
        return fallo(m, Estado::GRABANDO, "no se grabo audio (revisa el permiso de microfono)", Resultado{});

    r = ejecutar({"ffmpeg", "-y", "-loglevel", "error", "-i", m4a, "-ar", "16000", "-ac", "1",
                  "-c:a", "pcm_s16le", wav}, c.toCorto, err);
    m.conv = r.segundos;
    if (r.codigo != 0 || tamano(wav) < 3200)
        return fallo(m, Estado::GRABANDO, "no se pudo convertir el audio a WAV", r);

    // 2) TRANSCRIBIENDO
    mostrar(Estado::TRANSCRIBIENDO);
    r = ejecutar({c.whisperBin, "-m", c.whisperModelo, "-l", "es", "-f", wav, "-nt", "-np",
                  "-t", std::to_string(c.hilos)}, c.toSTT, err);
    m.stt = r.segundos;
    m.memStt = r.memKB;
    if (r.codigo != 0) return fallo(m, Estado::TRANSCRIBIENDO, "whisper fallo", r);
    std::string texto = sanitizarEntrada(r.salida);
    m.palEntrada = contarPalabras(texto);
    if (texto.empty()) {
        std::cout << "  No te escuche bien. Intenta de nuevo." << std::endl;
        m.resultado = "sin_voz";
        return true;
    }
    std::cout << "  Entendi: " << texto << std::endl;

    // 3) GENERANDO
    mostrar(Estado::GENERANDO);
    std::string prompt = "<|im_start|>system\n" + c.sistema + "<|im_end|>\n<|im_start|>user\n" +
                         texto + "<|im_end|>\n<|im_start|>assistant\n";
    std::vector<std::string> a = {c.llamaBin, "-m", c.llamaModelo, "-c", std::to_string(c.ctx),
                                  "-n", std::to_string(c.maxTokens), "-t", std::to_string(c.hilos),
                                  "--no-display-prompt"};
    for (const auto& e : c.llamaExtra) a.push_back(e);
    a.push_back("-p");
    a.push_back(prompt);

    r = ejecutar(a, c.toSLM, err);
    m.slm = r.segundos;
    m.memSlm = r.memKB;
    if (r.codigo != 0) return fallo(m, Estado::GENERANDO, "llama.cpp fallo", r);
    std::string respuesta = limpiarRespuesta(r.salida);
    m.palRespuesta = contarPalabras(respuesta);
    if (respuesta.empty()) return fallo(m, Estado::GENERANDO, "el modelo no devolvio texto", r);
    std::cout << "  Jarvis-son: " << respuesta << std::endl;

    // 4) HABLANDO
    mostrar(Estado::HABLANDO);
    r = ejecutar({"python", c.ttsScript, respuesta, resp}, c.toTTS, err);
    m.tts = r.segundos;
    m.memTts = r.memKB;
    if (r.codigo != 0 || tamano(resp) < 100) return fallo(m, Estado::HABLANDO, "la sintesis de voz fallo", r);

    double dur = duracionWav(resp);
    if (dur <= 0) dur = 5;
    t = Reloj::now();
    ejecutar({"termux-media-player", "play", resp}, c.toCorto, err);
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long>((dur + 0.6) * 1000)));
    ejecutar({"termux-media-player", "stop"}, c.toCorto, err);
    m.reprod = seg(t, Reloj::now());
    tcflush(STDIN_FILENO, TCIFLUSH);

    std::cout << std::fixed << std::setprecision(1) << "  Latencia hasta empezar a hablar: "
              << (m.conv + m.stt + m.slm + m.tts) << " s" << std::endl;
    return true;
}

// --- PROGRAMA PRINCIPAL ---
int main() {
    struct sigaction sa {};
    sa.sa_handler = alInterrumpir;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, nullptr);

    Config c = crearConfig();
    std::vector<std::string> faltan;
    for (const std::string& ruta : {c.whisperBin, c.whisperModelo, c.llamaBin, c.llamaModelo, c.ttsScript})
        if (!fs::exists(ruta)) faltan.push_back(ruta);
    for (const char* p : {"termux-microphone-record", "termux-media-player", "ffmpeg", "python"})
        if (!enPath(p)) faltan.push_back(std::string(p) + " (no esta en PATH)");
    if (!enPath("espeak-ng") && !enPath("espeak")) faltan.push_back("espeak-ng (no esta en PATH)");

    if (!faltan.empty()) {
        std::cerr << "Faltan componentes:\n";
        for (const auto& f : faltan) std::cerr << " - " << f << "\n";
        return 1;
    }

    std::cout << "Jarvis-son V1 - conversacion por turnos\n";
    int n = 0;
    std::string linea;
    while (!g_salir) {
        mostrar(Estado::LISTO);
        std::cout << "Enter = hablar | q + Enter = salir > " << std::flush;
        if (!std::getline(std::cin, linea)) break;
        if (linea == "q" || linea == "Q") break;

        std::error_code ec;
        fs::create_directories(c.home + "/.cache/jarvis-son", ec);
        std::string plantilla = c.home + "/.cache/jarvis-son/turno-XXXXXX";
        std::vector<char> buf(plantilla.begin(), plantilla.end());
        buf.push_back('\0');
        if (!mkdtemp(buf.data())) {
            std::cerr << "No se pudo crear la carpeta temporal.\n";
            break;
        }
        std::string dir = buf.data();

        Metricas m;
        m.turno = ++n;
        auto t0 = Reloj::now();
        bool ok = turno(c, m, dir);
        m.total = seg(t0, Reloj::now());
        if (!ok && m.resultado == "ok") m.resultado = "error";

        fs::remove_all(dir, ec);
        guardarMetricas(c, m);
    }
    std::cout << "\nHasta luego.\n";
    return 0;
}
