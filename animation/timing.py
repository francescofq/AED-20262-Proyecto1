#!/usr/bin/env python3
"""
timing.py - Calcula cuanto dura cada escena del video SIN renderizar.

Lee las tablas de tiempos de rbviz.py, los traces y el codigo de las escenas.
Sirve para dos cosas: cuadrar la duracion total antes de gastar un render de
20 minutos, y generar las marcas de tiempo del guion.

Uso:  python3 timing.py
"""
from __future__ import annotations

import json
import re
from pathlib import Path

AQUI = Path(__file__).resolve().parent
TRACES = AQUI.parent / "traces"

# ---------------------------------------------------------------------------
# Tablas de rbviz.py (se leen con regex para no tener que importar manim)
# ---------------------------------------------------------------------------
_src = (AQUI / "rbviz.py").read_text(encoding="utf-8")


def _dict(nombre: str) -> dict:
    m = re.search(rf"^{nombre} = (\{{.*?\}})", _src, re.S | re.M)
    if not m:
        raise SystemExit(f"No se encontro la tabla {nombre} en rbviz.py")
    return eval(m.group(1))


def _float(nombre: str) -> float:
    m = re.search(rf"^{nombre} = ([\d.]+)", _src, re.M)
    if not m:
        raise SystemExit(f"No se encontro {nombre} en rbviz.py")
    return float(m.group(1))


RUN_TIME = _dict("RUN_TIME")
HOLD = _dict("HOLD")
TEMPO_TRACE = _float("TEMPO_TRACE")
TEMPO_WAIT = _float("TEMPO_WAIT")

# Manim agrega un poco por escena (primer y ultimo fotograma, cierre de cada
# animacion). Medido comparando la prediccion con el render real.
OVERHEAD_POR_PARTE = 0.4


def costo_trace(nombre: str, speed: float, skip: set[str]) -> float:
    evs = json.loads((TRACES / f"{nombre}.json").read_text(encoding="utf-8"))["events"]
    total = 0.0
    for e in evs:
        a = e["action"]
        if a in skip:
            continue
        total += (RUN_TIME.get(a, 0.5) + HOLD.get(a, 0.0)) * speed
    return total * TEMPO_TRACE


# ---------------------------------------------------------------------------
# Analisis del codigo de una escena
# ---------------------------------------------------------------------------
def metodos(src: str) -> dict[str, str]:
    """Parte el archivo en cuerpos de metodo."""
    piezas = re.split(r"\n    def (\w+)\(", src)
    out = {}
    for i in range(1, len(piezas), 2):
        out[piezas[i]] = piezas[i + 1]
    return out


def orden_construct(cuerpo: str) -> list[str]:
    return re.findall(r"self\.(\w+)\(\)", cuerpo)


def costo_metodo(nombre: str, cuerpo: str, cuerpos: dict[str, str]) -> float:
    total = 0.0
    total += sum(float(x) for x in re.findall(r"run_time=([\d.]+)", cuerpo))
    total += sum(float(x) for x in re.findall(r"self\.wait\(([\d.]+)\)", cuerpo)) * TEMPO_WAIT

    trace = re.search(r'events_of\("(\w+)"\)', cuerpo)
    pt = re.search(r"self\.play_trace\([^)]*?speed=([\d.]+)(.*?)\)", cuerpo, re.S)
    if trace and pt:
        skip = set(re.findall(r'"(\w+)"', pt.group(2)))
        total += costo_trace(trace.group(1), float(pt.group(1)), skip)

    # caso_1 / caso_2 / caso_3 delegan en el helper _caso
    delega = re.search(r'self\._caso\(\s*\n?\s*"(\w+)"', cuerpo)
    if delega and "_caso" in cuerpos:
        helper = cuerpos["_caso"]
        total += sum(float(x) for x in re.findall(r"run_time=([\d.]+)", helper))
        total += sum(float(x) for x in re.findall(r"self\.wait\(([\d.]+)\)", helper)) * TEMPO_WAIT
        vel = re.search(r"speed: float = ([\d.]+)", helper)
        total += costo_trace(delega.group(1), float(vel.group(1)) if vel else 1.0, set())
    return total


def mmss(s: float) -> str:
    return f"{int(s // 60)}:{s % 60:05.2f}"


def main():
    print(f"TEMPO_TRACE={TEMPO_TRACE}  TEMPO_WAIT={TEMPO_WAIT}\n")
    acumulado = 0.0
    total_partes = []
    for n in (1, 2, 3):
        src = (AQUI / f"parte{n}.py").read_text(encoding="utf-8")
        cuerpos = metodos(src)
        secciones = orden_construct(cuerpos["construct"])
        print(f"PARTE {n}")
        dur_parte = 0.0
        for sec in secciones:
            if sec not in cuerpos:
                continue
            d = costo_metodo(sec, cuerpos[sec], cuerpos)
            print(f"   {mmss(acumulado):>8s}  {sec:22s} {d:6.1f} s")
            acumulado += d
            dur_parte += d
        acumulado += OVERHEAD_POR_PARTE
        dur_parte += OVERHEAD_POR_PARTE
        total_partes.append(dur_parte)
        print(f"   {'':8s}  {'subtotal':22s} {dur_parte:6.1f} s   ({mmss(dur_parte)})\n")

    print(f"TOTAL: {mmss(sum(total_partes))}")
    for i, d in enumerate(total_partes, 1):
        print(f"   Parte {i}: {mmss(d)}")


if __name__ == "__main__":
    main()
