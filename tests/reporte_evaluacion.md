# Reporte de Evaluación y Pruebas — Jarvis-son V1

## 1. Inventario del Dispositivo de Prueba
* *Modelo exacto:* Samsung Galaxy S23 Ultra (SM-S918B / dm3q)
* *Procesador (SoC):* Qualcomm Snapdragon 8 Gen 2 for Galaxy (SM8550 / qcom)
* *Memoria RAM total:* 12 GB (10 GiB utilizables por el sistema)
* *Versión de Android:* Android 17 (API / SDK 37)
* *Arquitectura:* ARM64 (aarch64)

## 2. Decisiones de Modelos (Adaptación al Hardware)
Dado que el dispositivo cuenta con 12 GB de RAM y arquitectura ARM64 de alto rendimiento (superando el mínimo de 3 GB de la guía), se optó por variantes más capaces dentro de la alternativa permitida por el plan:
* *STT:* whisper.cpp con modelo ggml-base (multilingüe).
* *SLM:* llama.cpp con Qwen2.5-1.5B-Instruct cuantizado en Q4_0 (contexto: 1024 tokens, salida máxima: 96 tokens, 4 hilos).
* *TTS:* sherpa-onnx ejecutando localmente la voz Piper vits-piper-es_MX-claude-high (22.05 kHz, español latinoamericano).

## 3. Prueba de Estabilidad y Modo Avión (Offline)
* *Turnos consecutivos:* *10 / 10 turnos completos* ejecutados en secuencia (LISTO -> GRABANDO -> TRANSCRIBIENDO -> GENERANDO -> HABLANDO -> LISTO) sin cierres por memoria (OOM) ni pérdida de estado.
* *Operación sin red:* Validado con el *Modo Avión activado* tras la descarga previa de los modelos; el flujo completo (STT, SLM y TTS) operó 100% local.

## 4. Métricas de Rendimiento

| Métrica | Valor Observado | Detalle |
| :--- | :---: | :--- |
| *Latencia hasta empezar a hablar (p50)* | *4.40 s* (4400 ms) | Mediana de los turnos evaluados |
| *Latencia hasta empezar a hablar (p95)* | *5.38 s* (5380 ms) | Percentil 95 de los turnos evaluados |
| *Tiempos individuales (10 turnos)* | 3.8 s – 5.7 s | 4.2s, 5.7s, 4.4s, 3.8s, 4.6s, 4.9s, 3.9s, 4.0s, 4.4s |
| *Desglose por etapa (STT / SLM / TTS)* | Ver logs/metricas.csv | Registrado automáticamente por el orquestador C++ (ru_maxrss y reloj monotónico) |

## 5. Evaluación de Calidad y Comportamiento Observado
* *Conjunto de pruebas:* Evaluado con tests/frases.txt (20 frases) y tests/preguntas.txt (10 preguntas cortas).
* *Aciertos destacados:* Respuestas rápidas, coherentes y en español neutro en preguntas factuales y consejos ("¿Cuál es la capital de Colombia?", "¿Qué planeta está más cerca del sol?", "¿Qué significa la sigla CPU?").
* *Errores y alucinaciones documentadas (Honestidad técnica):*
  * Error aritmético del SLM: Ante "¿Cuánto es 25 por 2?", el modelo Qwen2.5-1.5B Q4_0 respondió "125".
  * Ambigüedad fonética del STT: La expresión "lenguaje C++" fue transcrita fonéticamente por Whisper como "lenguaje se mas mas", provocando que el SLM interpretara la pregunta sobre el lenguaje humano en lugar de programación.

## 6. Privacidad, Seguridad y Limitaciones de la V1
* *Privacidad de audio:* El orquestador (src/main.cpp) crea un directorio temporal único (mkdtemp) por turno y ejecuta fs::remove_all() al finalizar o fallar, eliminando los archivos entrada.m4a, entrada.wav y respuesta.wav. No se almacenan conversaciones ni grabaciones en el repositorio.
* *Seguridad de ejecución:* Todas las herramientas externas (termux-microphone-record, ffmpeg, whisper-cli, llama-completion, python) se invocan mediante fork() + execvp() con vectores de argumentos separados, eliminando el riesgo de shell injection.
* *Limitaciones conocidas:* No soporta palabra de activación continua (wake word) ni interrupción mientras el asistente habla (no barge-in); la interacción es estrictamente por turnos iniciados con Enter.
