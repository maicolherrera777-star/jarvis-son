<img width="320" height="650" alt="instalacion_termux" src="https://github.com/user-attachments/assets/d2a45e4d-f16d-46a5-8a2e-c18257339195" />

Figura 1: Búsqueda e instalación del emulador de terminal Termux desde F-Droid en el dispositivo Android.

---

<img width="320" height="650" alt="actualizacion_repositorios" src="https://github.com/user-attachments/assets/36f5f7a9-6703-4af8-a053-d8daa927e0db" />

Figura 2: Actualización inicial de los repositorios y paquetes base del sistema en Termux (`pkg update && pkg upgrade -y`).

---

<img width="320" height="650" alt="instalacion_dependencias" src="https://github.com/user-attachments/assets/548f9ec4-d5e7-4fd5-91f1-a787ae914d4f" />

Figura 3: Instalación de herramientas de compilación y dependencias (`git`, `cmake`, `clang`, `make`, `ffmpeg`, `termux-api`, `python`).

---

<img width="320" height="650" alt="verificacion_hardware" src="https://github.com/user-attachments/assets/2c2345c0-5960-40ae-8ab3-39604eb67dfc" />

Figura 4: Verificación de especificaciones del dispositivo de prueba (`SM-S918B`, Android 17, arquitectura `aarch64`, memoria RAM y almacenamiento disponible).

---

<img width="320" height="650" alt="grabacion_conversion_audio" src="https://github.com/user-attachments/assets/9aa28907-6421-4c68-8626-e1fd4b96d3b8" />


Figura 5: Prueba inicial de captura de micrófono (`termux-microphone-record`) y conversión de audio a WAV 16 kHz mono mediante `ffmpeg`.

---

<img width="320" height="650" alt="verificacion_reproduccion_audio" src="https://github.com/user-attachments/assets/a5f8f7d9-607b-4ebd-aa49-d501a2d2beb9" />

Figura 6: Verificación del formato de audio (`16000 Hz`, mono, `pcm_s16le`), prueba de reproducción con `termux-media-player` y limpieza de archivos temporales.

---

<img width="320" height="650" alt="estructura_clonacion_whisper" src="https://github.com/user-attachments/assets/7d72776c-38f0-43df-aad6-7ba6e88853b1" />

Figura 7: Creación de la estructura de directorios de `jarvis-son` y clonación del repositorio `whisper.cpp`.

---

<img width="320" height="650" alt="configuracion_cmake_whisper" src="https://github.com/user-attachments/assets/2e77a248-4d33-4d05-9057-343fd6564659" />

Figura 8: Configuración de compilación con CMake para `whisper.cpp` detectando las extensiones de aceleración del procesador ARM64 (`dotprod`, `i8mm`, `FP16`).

---

<img width="320" height="650" alt="compilacion_whisper_cli" src="https://github.com/user-attachments/assets/9b5f0fd6-84d6-4133-9741-f76023604629" />

Figura 9: Finalización de la compilación de `whisper.cpp` al 100% y verificación del binario `whisper-cli` en `build/bin/`.

---

<img width="320" height="650" alt="prueba_whisper_base" src="https://github.com/user-attachments/assets/4a894f0f-ac88-4d8b-acb4-62f9f7a03ec4" />

Figura 10: Prueba de transcripción en español con el modelo `ggml-base.bin` (147.37 MB), logrando reconocer la frase completa con marcas de tiempo en 1.40 s.

---

<img width="320" height="650" alt="comparativa_whisper_tiny" src="https://github.com/user-attachments/assets/f5fd644d-2431-4179-9a1e-cb1f5d07aabf" />

Figura 11: Prueba comparativa con `ggml-tiny.bin` (77.11 MB); aunque es ligeramente más rápido (1.24 s), introduce errores fonéticos en español (`"Pola"` en vez de `"Hola"`).

---

<img width="320" height="650" alt="whisper_base_sin_timestamps" src="https://github.com/user-attachments/assets/b2656ace-dc4f-4f1d-a6f4-7a3f65b9c60b" />

Figura 12: Ejecución optimizada de `ggml-base.bin` con el parámetro `-nt` (sin marcas de tiempo), reduciendo la latencia de transcripción a 1.18 s y limpiando la salida de texto.

---

<img width="320" height="650" alt="corpus_frases_stt" src="https://github.com/user-attachments/assets/4f5267a5-e4b4-41f5-bf2e-877d70a1765f" />

Figura 13: Creación del conjunto de 20 frases de prueba en español (`tests/frases.txt`) para evaluar la tasa de error de palabras (WER) del módulo de voz.

---

<img width="320" height="650" alt="clonacion_llama_cpp" src="https://github.com/user-attachments/assets/953f639c-4dbd-4b06-8cbb-6378f5a520ea" />

Figura 14: Clonación del repositorio oficial `llama.cpp` para la inferencia local del modelo de lenguaje (SLM).

---

<img width="320" height="650" alt="configuracion_cmake_llama" src="https://github.com/user-attachments/assets/d8119165-c693-46b3-937c-93ed6878583c" />

Figura 15: Configuración de CMake para `llama.cpp` con soporte de red desactivado (`-DLLAMA_CURL=OFF`) y detección de extensiones ARM64 (`dotprod`, `i8mm`, OpenMP).

---

<img width="320" height="650" alt="compilacion_llama_cli" src="https://github.com/user-attachments/assets/cd99ec40-9428-4f1d-9906-f5877857cfd2" />

Figura 16: Verificación de los ejecutables compilados de `llama.cpp` en `build/bin/`, confirmando la disponibilidad del binario `llama-cli`.

---

<img width="320" height="650" alt="descarga_verificacion_qwen" src="https://github.com/user-attachments/assets/1ce90835-f481-4fc0-b9d1-d0ef2ce29b4a" />

Figura 17: Descarga del modelo cuantizado `qwen2.5-1.5b-instruct-q4_0.gguf` (1017 MB) y verificación de integridad mediante suma de comprobación `sha256sum`.

---

<img width="370" height="650" alt="prueba_inferencia_llama" src="https://github.com/user-attachments/assets/90db28a6-b8a6-4f9e-8a7b-ae5ff21acb92" />

Figura 18: Primera prueba de inferencia local en español con `llama-cli` y Qwen 2.5 1.5B, alcanzando una velocidad de generación de 33.9 tokens/s.

---

<img width="320" height="650" alt="medicion_memoria_ram_llm" src="https://github.com/user-attachments/assets/3881e363-5f4f-46e4-bd29-0252f08851fd" />

Figura 19: Medición de consumo máximo de memoria RAM durante la inferencia del modelo Qwen 2.5 1.5B mediante la utilidad `time -v` (`Maximum resident set size: 2001076 kB` ≈ 1.95 GB).

---

<img width="320" height="650" alt="image" src="https://github.com/user-attachments/assets/0d0d328e-40be-4fe6-8bca-0bbf54128206" />

Figura 20: Creación del conjunto de 10 preguntas de prueba (`tests/preguntas.txt`) para evaluar la coherencia y tiempos de respuesta del modelo de lenguaje.

---

<img width="320" height="650" alt="manifiesto_modelo_llm" src="https://github.com/user-attachments/assets/da85a5a2-a028-4752-8e77-374b4b2efb7d" />

Figura 21: Creación del archivo de manifiesto (`models/manifest.json`) con los metadatos de trazabilidad, licencia y suma SHA-256 del modelo Qwen 2.5 1.5B.

---

<img width="320" height="650" alt="instalacion_voz_piper_sherpa" src="https://github.com/user-attachments/assets/4b618317-0bce-48d9-91bf-a4003bb176c9" />

Figura 22: Descarga, extracción y verificación SHA-256 del modelo de voz en español (`vits-piper-es_MX-claude-high`) junto con la instalación de `sherpa-onnx` en Python.

---

<img width="320" height="650" alt="instalacion_espeak_tts" src="https://github.com/user-attachments/assets/116d4960-a34b-4fed-a31c-884425244e15" />

Figura 23: Instalación del motor fonético `espeak` (`pkg install espeak -y`), requerido para la conversión de texto en español a fonemas IPA.

---

<img width="320" height="650" alt="codigo_script_tts" src="https://github.com/user-attachments/assets/23a3aaac-6bb2-47ce-98c9-15d34f27219f" />

Figura 24: Implementación del módulo `scripts/tts.py` con segmentación por pausas, fonetización (`espeak -v es-419 --ipa`) y síntesis local mediante `sherpa-onnx`.

---

<img width="320" height="650" alt="pruebas_rendimiento_tts" src="https://github.com/user-attachments/assets/43c88618-3c98-426a-9e43-f722a118f01f" />

Figura 25: Pruebas de ejecución de `scripts/tts.py` verificando limpieza de formato Markdown/emojis, generación de audio en 0.19–0.48 s (`RTF: 0.069`) y metadatos del modelo `es_MX-claude-high`.

---

<img width="320" height="650" alt="manifiesto_integral_modelos" src="https://github.com/user-attachments/assets/67c79636-42a0-4f28-b27b-68235026a895" />

Figura 26: Archivo de manifiesto integral (`models/manifest.json`) documentando las versiones, licencias y sumas de verificación SHA-256 de los tres módulos (STT, SLM y TTS).

---

<img width="780" height="410" alt="token_acceso_github" src="https://github.com/user-attachments/assets/045f40ac-4655-4321-bc20-f89e1999b8c5" />

Figura 27: Creación de un Personal Access Token (classic) en GitHub con alcance `repo` para autenticar las operaciones de sincronización (`git push`) desde el entorno móvil.

---

<img width="320" height="650" alt="control_versiones_git_tts" src="https://github.com/user-attachments/assets/a6a8f3bb-75fe-4971-82f6-a51492d0adf4" />

Figura 28: Flujo de integración continua en Git desde Termux: creación de la rama `feat/tts`, confirmación de los módulos de audio/manifiesto y subida al repositorio remoto en GitHub.

---

<img width="780" height="410" alt="inferencia_limpia_llama_completion" src="https://github.com/user-attachments/assets/e465791e-9a6d-4ec0-aaa5-f82cf7ee6654" />

Figura 29: Ejecución en modo no interactivo mediante `llama-completion` y formato ChatML (`<|im_start|>`), obteniendo una respuesta de texto limpia sin registros de consola para alimentar directamente al módulo TTS.

---

<img width="320" height="650" alt="compilacion_ejecucion_orquestador" src="https://github.com/user-attachments/assets/9209715b-8f77-44d8-bc50-dfae170598c6" />

Figura 30: Compilación del orquestador principal en C++ (`src/main.cpp`, 454 líneas) y primera ejecución del flujo completo de conversación por turnos (`[LISTO]`, `[GRABANDO]`, `[TRANSCRIBIENDO]`, `[GENERANDO]`, `[HABLANDO]`).

---

<img width="320" height="650" alt="pruebas_conversacion_extremo_a_extremo" src="https://github.com/user-attachments/assets/bbbb4330-f197-4385-853c-ffbe43a04da8" />

Figura 31: Pruebas de conversación continua de extremo a extremo con `jarvis-son`, registrando tiempos de latencia total hasta el inicio del habla entre 3.8 s y 5.7 s.

---

<img width="320" height="650" alt="pruebas_manejo_errores_resiliencia" src="https://github.com/user-attachments/assets/a5a84608-581d-490d-b8cb-bebf4cce5b21" />

Figura 32: Pruebas de tolerancia a fallos del orquestador C++: detección de audio vacío sin transcripción y captura controlada de error en la etapa `[HABLANDO]` al simular la ausencia de los archivos de voz.

---

<img width="320" height="650" alt="analisis_estadistico_metricas" src="https://github.com/user-attachments/assets/a7605ca4-9f0a-4418-bd37-edd9651fed4a" />

Figura 33: Análisis estadístico en Python sobre `logs/metricas.csv` (16 turnos), reportando medianas (`p50`), percentiles (`p95`) de latencia por etapa (`t_hasta_voz_s` p50 = 4.51 s) y consumo pico de memoria RAM por componente.

---

<img width="320" height="650" alt="integracion_final_rama_main" src="https://github.com/user-attachments/assets/a6a82fed-4313-4dff-aef4-237b18f1f0be" />

Figura 34: Sincronización final del repositorio en la rama `main` tras integrar `feat/orchestrator`, verificando la estructura completa de archivos y un árbol de trabajo limpio (`working tree clean`).
