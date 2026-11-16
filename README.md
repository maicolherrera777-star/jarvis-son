# Jarvis-son V1 — Asistente de Voz Offline en Android

Asistente de voz conversacional por turnos ejecutado 100% de forma local sobre Termux en Android ARM64, desarrollado para la asignatura de Inteligencia Artificial (COTECNOVA).

## 1. Arquitectura y Flujo por Turnos

El orquestador en C++ (src/main.cpp) ejecuta cada etapa de forma estrictamente secuencial (sin escucha continua ni interrupción durante el habla), validando códigos de salida y limpiando recursos temporales al finalizar cada turno[cite: 7, 25]:

```text
[LISTO] -> [GRABANDO] -> [TRANSCRIBIENDO] -> [GENERANDO] -> [HABLANDO] -> [LISTO]
