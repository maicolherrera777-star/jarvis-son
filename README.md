# Jarvis-son V1 — Asistente de Voz Offline en Android
Asistente de voz conversacional por turnos ejecutado 100% de forma local sobre Termux en Android ARM64.

## 1. Arquitectura y Flujo por Turnos
```text
[LISTO] -> [GRABANDO] -> [TRANSCRIBIENDO] -> [GENERANDO] -> [HABLANDO] -> [LISTO]
```

## 2. Componentes del Stack Local
* **Orquestador:** C++17 (`src/main.cpp`).
* **STT / LLM / TTS:** `whisper.cpp`, `llama.cpp` cuantizado y `termux-tts-speak`.

## 3. Métricas de Rendimiento (Latencia por Turno)

| Etapa del Pipeline | Latencia p50 | Latencia p95 |
| :--- | :---: | :---: |
| **Latencia hasta empezar a hablar** | **`4.40 s` (`4400 ms`)** | **`5.38 s` (`5380 ms`)** |
