# Pruebas y Resultados - Jarvis-son V1

## 1. Equipo utilizado para las pruebas
- **Celular:** Samsung Galaxy S23 Ultra (SM-S918B / dm3q)
- **Procesador:** Qualcomm Snapdragon 8 Gen 2 (SM8550)
- **RAM:** 12 GB (10 GiB libres para el sistema)
- **Sistema:** Android 17 (SDK 37)
- **Arquitectura:** ARM64 (aarch64)

## 2. Configuración de modelos elegida
Como el celular de prueba tiene 12 GB de RAM y procesador ARM64, pudimos configurar los modelos permitidos en una versión un poco más completa sin que la aplicación se cerrara:
- **Reconocimiento de voz (STT):** whisper.cpp usando el modelo ggml-base.
- **Modelo de lenguaje (SLM):** llama.cpp con Qwen2.5-1.5B-Instruct (Q4_0), límite de contexto en 1024 tokens, respuesta máxima de 96 tokens y 4 hilos de ejecución.
- **Síntesis de voz (TTS):** sherpa-onnx con la voz en español vits-piper-es_MX-claude-high (22.05 kHz).

## 3. Estabilidad y funcionamiento sin internet
- **Prueba de 10 turnos:** Se ejecutaron 10 preguntas seguidas pasando por todos los estados (LISTO -> GRABANDO -> TRANSCRIBIENDO -> GENERANDO -> HABLANDO -> LISTO) sin bloqueos ni cierres por falta de memoria.
- **Modo Avión:** Después de descargar los modelos, activamos el Modo Avión en el teléfono y verificamos que todo el proceso (escuchar, procesar y responder con voz) funciona sin conexión a internet.

## 4. Tiempos de respuesta medidos

| Medición | Resultado | Observación |
| :--- | :---: | :--- |
| **Tiempo hasta empezar a hablar (p50)** | **4.40 s** (`4400 ms`) | Mediana de los turnos probados |
| **Tiempo hasta empezar a hablar (p95)** | **5.38 s** (`5380 ms`) | Percentil 95 |
| **Rango en 10 turnos** | 3.8 s a 5.7 s | 4.2s, 5.7s, 4.4s, 3.8s, 4.6s, 4.9s, 3.9s, 4.0s, 4.4s |
| **Detalle por etapa** | logs/metricas.csv | Guardado automáticamente por main.cpp durante la ejecución |

## 5. Pruebas de calidad y fallos encontrados
Probamos el asistente usando las listas tests/frases.txt (20 frases) y tests/preguntas.txt (10 preguntas cortas).

- **Lo que funcionó bien:** Responde rápido y de forma coherente a preguntas directas de conocimiento general (por ejemplo, *"¿Cuál es la capital de Colombia?"*, *"¿Qué planeta está más cerca del sol?"* o *"¿Qué significa la sigla CPU?"*).
- **Fallos detectados en las pruebas:**
  - *Cálculo matemático:* Al preguntarle *"¿Cuánto es 25 por 2?"*, el modelo Qwen2.5-1.5B respondió "125", mostrando que al ser un modelo pequeño comprimido a 4 bits pierde precisión en operaciones numéricas.
  - *Confusión fonética en el audio:* Cuando dijimos *"lenguaje C++"*, Whisper transcribió "lenguaje se mas mas", lo que hizo que el modelo respondiera sobre idiomas humanos en lugar de programación.

## 6. Manejo de archivos temporales y seguridad
- **Borrado de audios:** En cada turno src/main.cpp crea una carpeta temporal con mkdtemp para guardar entrada.m4a, entrada.wav y respuesta.wav. Apenas termina el turno (o si ocurre un fallo), fs::remove_all() elimina la carpeta para no dejar grabaciones guardadas en el teléfono.
- **Llamada a procesos externos:** Para ejecutar termux-microphone-record, ffmpeg, whisper-cli, llama-completion y python usamos fork() junto con execvp() pasando los argumentos por separado, evitando problemas de inyección de comandos en la consola.
- **Limitaciones actuales:** El asistente funciona únicamente por turnos iniciados con Enter; todavía no cuenta con palabra de activación por voz ni permite interrumpirlo mientras está hablando.
