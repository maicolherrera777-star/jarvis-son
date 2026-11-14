import os, re, sys, glob, time, wave, array, shutil, subprocess
import sherpa_onnx

VOZ = os.path.expanduser("~/voces/vits-piper-es_MX-claude-high")
PAUSAS = {",": 0.15, ";": 0.25, ":": 0.25, ".": 0.40, "!": 0.40, "?": 0.40, "\n": 0.40}

def limpiar(t):
    t = re.sub(r"[^\w\s.,;:!?\-]", " ", t)
    t = t.replace("_", " ")
    t = re.sub(r"(^|\s)-+", r"\1", t)
    return t.strip()

def segmentos(t):
    partes = re.split(r"([.,;:!?\n]+)", t)
    res = []
    for i in range(0, len(partes), 2):
        frase = partes[i].strip().lstrip("-")
        sep = partes[i + 1][0] if i + 1 < len(partes) and partes[i + 1] else None
        if frase:
            res.append((frase, PAUSAS.get(sep, 0.0) if sep else 0.0))
    return res

def main():
    if len(sys.argv) < 3:
        print("Uso: python tts.py 'texto' salida.wav")
        sys.exit(1)
    texto = limpiar(sys.argv[1])
    salida = sys.argv[2]
    segs = segmentos(texto)
    if not segs:
        print("ERROR: no hay texto para leer.")
        sys.exit(3)

    espeak = shutil.which("espeak-ng") or shutil.which("espeak")
    if not espeak:
        print("ERROR: instala espeak-ng (pkg install espeak-ng)")
        sys.exit(1)

    config = sherpa_onnx.OfflineTtsConfig(
        model=sherpa_onnx.OfflineTtsModelConfig(
            vits=sherpa_onnx.OfflineTtsVitsModelConfig(
                model=glob.glob(VOZ + "/*.onnx")[0],
                tokens=VOZ + "/tokens.txt",
                data_dir=VOZ + "/espeak-ng-data",
            ),
            num_threads=4,
            provider="cpu",
        ),
    )
    t0 = time.time()
    tts = sherpa_onnx.OfflineTts(config)
    t_carga = time.time() - t0

    muestras = []
    tasa = None
    t1 = time.time()
    for frase, pausa in segs:
        ipa = subprocess.check_output(
            [espeak, "-v", "es-419", "-q", "--ipa", frase], text=True
        )
        ipa = " ".join(ipa.split())
        if not ipa:
            continue
        cfg = sherpa_onnx.GenerationConfig()
        cfg.sid = 0
        cfg.speed = 1.0
        cfg.phoneme_codepoints = [[ord(c) for c in ipa]]
        audio = tts.generate(frase, cfg)
        if len(audio.samples) == 0:
            continue
        tasa = audio.sample_rate
        muestras.extend(audio.samples)
        muestras.extend([0.0] * int(tasa * pausa))
    t_gen = time.time() - t1

    if not muestras:
        print("ERROR: el motor no genero audio.")
        sys.exit(2)

    duracion = len(muestras) / tasa
    pcm = array.array("h", [int(max(-1.0, min(1.0, s)) * 32767) for s in muestras])
    with wave.open(salida, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(tasa)
        w.writeframes(pcm.tobytes())

    print(f"Carga del modelo: {t_carga:.2f} s")
    print(f"Generacion:       {t_gen:.2f} s")
    print(f"Duracion audio:   {duracion:.2f} s")
    print(f"RTF:              {t_gen / duracion:.3f}")
    print(f"Archivo:          {salida}")

main()
