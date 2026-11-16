# Jarvis-son V1 — Asistente de Voz Offline en Android

Asistente de voz conversacional por turnos ejecutado 100% de forma local (sin conexión a internet) sobre Termux en Android ARM64.

## 1. Arquitectura y Flujo por Turnos
El sistema opera mediante una máquina de estados en C++ (`src/main.cpp`) que coordina cada etapa de forma secuencial sin solapamiento ni escucha continua (*barge-in* fuera de V1):

```text
[LISTO] -> [GRABANDO] -> [TRANSCRIBIENDO] -> [GENERANDO] -> [HABLANDO] -> [LISTO]
                                                                    \-> [ERROR]
```

## 2. Componentes del Stack Local
* **Orquestador:** C++17 compilado con CMake (`src/main.cpp`).
* **Captura de Audio (STT):** `termux-microphone-record` + `whisper.cpp` (ejecución local en CPU ARM64).
* **Modelo de Lenguaje (LLM):** `llama.cpp` cuantizado corriendo en local.
* **Síntesis de Voz (TTS):** `termux-tts-speak` nativo de Termux:API.

## 3. Métricas de Rendimiento (Latencia por Turno)
Mediciones reales tomadas en dispositivo Android ARM64 bajo Termux:

| Etapa del Pipeline | Latencia p50 | Latencia p95 | Notas |
| :--- | :---: | :---: | :--- |
| **Grabación (`GRABANDO`)** | Manual | Manual | Controlada por turno del usuario |
| **Transcripción (`TRANSCRIBIENDO`)** | *-- ms* | *-- ms* | `whisper.cpp` local |
| **Inferencia LLM (`GENERANDO`)** | *-- ms* | *-- ms* | `llama.cpp` local |
| **Síntesis TTS (`HABLANDO`)** | *-- ms* | *-- ms* | `termux-tts-speak` |
| **Total End-to-End** | ***-- ms*** | ***-- ms*** | Tiempo total sin incluir voz del usuario |

## 4. Requisitos e Instalación en Termux
1. Instalar dependencias base en Termux:
   ```bash
   pkg update && pkg upgrade -y
   pkg install -y git cmake clang make termux-api
   ```
2. Clonar el repositorio y compilar:
   ```bash
   git clone https://github.com
   cd jarvis-son
   cmake -B build
   cmake --build build -j4
   ```

## 5. Ejecución
```bash
./build/jarvis-son
```
* Presiona **Enter** para iniciar un turno de conversación.
* Escribe **`q` + Enter** para salir de forma limpia.
