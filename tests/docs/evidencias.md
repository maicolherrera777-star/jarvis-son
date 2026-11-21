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

