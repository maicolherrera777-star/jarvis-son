import os, sys, glob, shutil, subprocess, time, wave, array
import sherpa_onnx

if len(sys.argv) < 3:
    print("Uso: python tts_fonemas.py 'texto' prefijo_salida")
    sys.exit(1)

texto = sys.argv[1]
prefijo = sys.argv[2]

voz = os.path.expanduser("~/voces/vits-piper-es_MX-claude-high")
modelo = glob.glob(voz + "/*.onnx")[0]

config = sherpa_onnx.OfflineTtsConfig(
    model=sherpa_onnx.OfflineTtsModelConfig(
        vits=sherpa_onnx.OfflineTtsVitsModelConfig(
            model=modelo,
            tokens=voz + "/tokens.txt",
            data_dir=voz + "/espeak-ng-data",
        ),
        num_threads=4,
        provider="cpu",
    ),
)
tts = sherpa_onnx.OfflineTts(config)

exe = shutil.which("espeak-ng") or shutil.which("espeak")
if not exe:
    print("ERROR: instala espeak-ng (pkg install espeak-ng)")
    sys.exit(1)

ipa = subprocess.check_output([exe, "-v", "es-419", "-q", "--ipa", texto], text=True)
ipa = " ".join(ipa.split()).strip()
print("Fonemas:", ipa)

# Simbolos que la voz conoce (tokens.txt)
conocidos = set()
for linea in open(voz + "/tokens.txt", encoding="utf-8"):
    linea = linea.rstrip("\n")
    if " " in linea:
        conocidos.add(linea.rsplit(" ", 1)[0])
faltan = sorted({c for c in ipa if c not in conocidos})
print("Simbolos que NO estan en tokens.txt:", faltan)

variantes = {}
variantes["A_simple"] = [ord(c) for c in ipa]
b = [ord("^"), ord("_")]
for c in ipa:
    b += [ord(c), ord("_")]
b.append(ord("$"))
variantes["B_con_pads"] = b

def guardar(muestras, tasa, ruta):
    pcm = array.array("h", [int(max(-1.0, min(1.0, s)) * 32767) for s in muestras])
    with wave.open(ruta, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(tasa)
        w.writeframes(pcm.tobytes())

for nombre, cps in variantes.items():
    print("\n--- Variante", nombre, "---")
    cfg = sherpa_onnx.GenerationConfig()
    cfg.sid = 0
    cfg.speed = 1.0
    cfg.phoneme_codepoints = [cps]
    audio = None
    t = time.time()
    for forma, llamada in (
        ("generate(texto, cfg)", lambda: tts.generate(texto, cfg)),
        ("generate(texto, gen_config=cfg)", lambda: tts.generate(texto, gen_config=cfg)),
    ):
        try:
            audio = llamada()
            print("Forma que funciono:", forma)
            break
        except Exception as e:
            print("Fallo", forma, "->", str(e)[:400])
    if audio is None or len(audio.samples) == 0:
        print("Sin audio en esta variante")
        continue
    t_gen = time.time() - t
    dur = len(audio.samples) / audio.sample_rate
    ruta = f"{prefijo}_{nombre}.wav"
    guardar(audio.samples, audio.sample_rate, ruta)
    print(f"Generacion: {t_gen:.2f} s | Duracion: {dur:.2f} s | RTF: {t_gen/dur:.3f}")
    print("Archivo:", ruta)
