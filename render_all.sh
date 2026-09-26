#!/usr/bin/env bash
# ---------------------------------------------------------------------------
#  render_all.sh - Pipeline completo: C++ -> traces -> 3 videos -> video final
#
#  Uso:
#     ./render_all.sh            calidad alta (1920x1080, 60 fps)  [entrega]
#     ./render_all.sh --fast     calidad baja (854x480, 15 fps)    [previsualizar]
# ---------------------------------------------------------------------------
set -euo pipefail
cd "$(dirname "$0")"

QUALITY="-qh"; TAG="1080p60"
if [[ "${1:-}" == "--fast" ]]; then QUALITY="-ql"; TAG="480p15"; fi

PY=animation/.venv/bin/python
if [[ ! -x "$PY" ]]; then
  echo "ERROR: falta el entorno virtual. Crealo con:"
  echo "  uv venv --python 3.12 animation/.venv"
  echo "  uv pip install --python animation/.venv/bin/python -r animation/requirements.txt"
  exit 1
fi

echo "==> [1/3] Compilando la implementacion en C++ y generando los traces"
g++ -std=c++20 -O2 main.cpp -o rbtree
./rbtree --quiet

echo "==> [2/3] Renderizando las 3 partes ($TAG)"
cd animation
for p in 1 2 3; do
  echo "    Parte $p..."
  "../$PY" -m manim render "$QUALITY" "parte${p}.py" "Parte${p}"
done
cd ..

echo "==> [3/3] Uniendo las 3 partes en video_final.mp4"
FFMPEG=$("$PY" -c "import imageio_ffmpeg; print(imageio_ffmpeg.get_ffmpeg_exe())")
LIST=$(mktemp); trap 'rm -f "$LIST"' EXIT
for p in 1 2 3; do
  PART="animation/media/videos/parte${p}/${TAG}/Parte${p}.mp4"
  [[ -f "$PART" ]] || { echo "ERROR: no se genero $PART"; exit 1; }
  echo "file '$(realpath "$PART")'" >> "$LIST"
done
# Las 3 partes salen del mismo renderer: misma resolucion, fps y codec,
# asi que se pueden concatenar copiando streams (sin recodificar).
"$FFMPEG" -y -f concat -safe 0 -i "$LIST" -c copy video_final.mp4 -loglevel error

echo
echo "Listo."
for p in 1 2 3; do
  echo "  Parte $p : animation/media/videos/parte${p}/${TAG}/Parte${p}.mp4"
done
echo "  FINAL   : video_final.mp4"
"$FFMPEG" -i video_final.mp4 2>&1 | grep -E "Duration|Stream #0:0" | sed 's/^/  /'
