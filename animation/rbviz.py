"""
rbviz.py - Capa de visualizacion para los traces del Red-Black Tree.

Este modulo NO implementa la estructura de datos: solo dibuja los eventos que
emitio la implementacion en C++ (traces/*.json). La posicion de cada nodo viene
de los campos 'depth' y 'order' que el propio arbol calculo recorriendo su
memoria, y el color viene del campo 'color' del nodo real. Si el trace no
existe, la animacion no puede dibujar nada: es una dependencia dura y a
proposito.
"""
from __future__ import annotations

import json
import textwrap
from pathlib import Path

import numpy as np
from manim import *

ROOT = Path(__file__).resolve().parent.parent
TRACES = ROOT / "traces"


def load_trace(name: str) -> dict:
    path = TRACES / f"{name}.json"
    if not path.exists():
        raise FileNotFoundError(
            f"No existe {path}. Genera los traces primero:\n"
            f"  g++ -std=c++20 -O2 main.cpp -o rbtree && ./rbtree --quiet"
        )
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def events_of(name: str) -> list[dict]:
    return load_trace(name)["events"]


def load_stats() -> list[dict]:
    with open(TRACES / "p3_stats.json", encoding="utf-8") as fh:
        return json.load(fh)["comparacion"]


def last_op(events: list[dict]) -> list[dict]:
    """Solo los eventos de la ultima operacion del trace."""
    starts = [i for i, e in enumerate(events) if e["action"] == "op_begin"]
    return events[starts[-1]:] if starts else events


def until_last_op(events: list[dict]) -> list[dict]:
    starts = [i for i, e in enumerate(events) if e["action"] == "op_begin"]
    return events[:starts[-1]] if starts else []


# --------------------------------------------------------------------------
# Paleta
# --------------------------------------------------------------------------
BG         = "#0E1015"
RED_FILL   = "#D93F3F"
RED_STROKE = "#FF9A95"
BLK_FILL   = "#23262F"
BLK_STROKE = "#8D93A6"
EDGE       = "#6E748A"
ACCENT     = "#F2C14E"
ACCENT2    = "#5AA9E6"
OK         = "#5BC98C"
WARN       = "#F2A14E"
TXT        = "#E8EAF0"
MUTED      = "#979DB0"

FOCUS_ACTIONS = {"compare", "attach", "recolor", "rotate_left", "rotate_right",
                 "fixup_case", "root_black", "found", "target", "successor",
                 "transplant", "double_black"}

RUN_TIME = {
    "op_begin": 0.45, "compare": 0.45, "attach": 0.65, "fixup_case": 0.95,
    "recolor": 0.60, "rotate_left": 1.00, "rotate_right": 1.00,
    "root_black": 0.55, "target": 0.55, "successor": 0.70, "transplant": 0.75,
    "double_black": 0.85, "found": 0.70, "not_found": 0.70, "op_end": 0.40,
}
HOLD = {"fixup_case": 0.35, "op_end": 0.20, "found": 0.30, "not_found": 0.30}

# --------------------------------------------------------------------------
# Ritmo global. Se ajustan estos dos numeros para cuadrar la duracion total
# del video sin tocar ninguna escena.
#   TEMPO_TRACE -> velocidad de la animacion del arbol
#   TEMPO_WAIT  -> pausas de lectura (todos los self.wait de las escenas)
# --------------------------------------------------------------------------
TEMPO_TRACE = 1.20
TEMPO_WAIT = 1.00


def wrap(text: str, width: int = 74) -> str:
    return "\n".join(textwrap.wrap(text, width=width)) or " "


def grid(rows: list[list[Mobject]], col_buff: float = 0.60,
         row_buff: float = 0.40, aligns: list | None = None) -> VGroup:
    """Tabla con columnas de ancho MEDIDO, no con coordenadas X a ojo.

    Poner cada columna en una X fija hace que un texto largo ("eliminacion")
    se monte sobre la columna siguiente. Aqui se mide el ancho real de cada
    columna y se reserva ese espacio, asi nunca se solapan.
    """
    ncols = max(len(r) for r in rows)
    widths = [max((r[c].width for r in rows if c < len(r)), default=0.0)
              for c in range(ncols)]

    xs, x = [], 0.0
    for w in widths:
        xs.append(x)
        x += w + col_buff
    total = x - col_buff

    row_h = max(m.height for r in rows for m in r) + row_buff

    group = VGroup()
    for i, cells in enumerate(rows):
        row = VGroup()
        for c, mob in enumerate(cells):
            align = aligns[c] if aligns and c < len(aligns) else LEFT
            left = xs[c] - total / 2.0
            if align is RIGHT:
                mob.move_to([left + widths[c], 0, 0], aligned_edge=RIGHT)
            elif align is ORIGIN:
                mob.move_to([left + widths[c] / 2.0, 0, 0])
            else:
                mob.move_to([left, 0, 0], aligned_edge=LEFT)
            mob.set_y(-i * row_h)
            row.add(mob)
        group.add(row)
    return group


def fit_width(mob: Mobject, max_width: float = 12.8) -> Mobject:
    """Reduce el texto si no cabe en el cuadro.

    Manim no ajusta ni corta: un Text mas ancho que la pantalla simplemente se
    sale por los bordes o se monta sobre lo que tenga al lado. Esto lo evita.
    """
    if mob.width > max_width:
        mob.scale_to_fit_width(max_width)
    return mob


def swap(holder: VGroup, *mobjects: Mobject) -> VGroup:
    """Reemplaza el contenido de un contenedor sin interpolar (cambio seco)."""
    if holder.submobjects:
        holder.remove(*list(holder.submobjects))
    if mobjects:
        holder.add(*mobjects)
    return holder


# --------------------------------------------------------------------------
# Vista del arbol
# --------------------------------------------------------------------------
class TreeView:
    def __init__(self, scene: Scene, span: float = 11.4, top: float = 1.70,
                 row: float = 1.10, radius: float = 0.33, max_col: float = 1.50):
        self.scene = scene
        self.span, self.top, self.row = span, top, row
        self.radius, self.max_col = radius, max_col

        self.nodes: dict[int, VGroup] = {}
        self.links: list[tuple[int, int]] = []
        self.node_layer = VGroup()
        self.edge_layer = always_redraw(self._build_edges)
        scene.add(self.edge_layer, self.node_layer)

    def fit(self, events: list[dict], bottom: float = -1.75, top: float = 2.05):
        """Fija la escala UNA vez, mirando el arbol mas grande de todo el trace.

        Se hace antes de animar para que la disposicion no salte de frame en
        frame: el espaciado se calcula con la profundidad y el ancho maximos
        que el arbol llegara a tener en esta escena.
        """
        # bottom = -1.75 deja el borde inferior del nodo mas profundo en
        # -2.08, con holgura sobre el rotulo que vive en -2.50. Antes se
        # tocaban y parecia que los nodos se montaban sobre el texto.
        depths = [s["depth"] for e in events for s in e["nodes"]]
        widths = [max((s["order"] for s in e["nodes"]), default=0) + 1
                  for e in events]
        max_depth = max(depths, default=0)
        max_cols = max(widths, default=1)
        if max_depth > 0:
            self.row = min(1.10, (top - bottom) / max_depth)
        if max_cols > 1:
            self.max_col = min(1.50, self.span / (max_cols - 1))
        # Centra verticalmente el arbol mas alto dentro del area disponible,
        # para que un arbol de 2 nodos no quede pegado al borde superior.
        centro = (top + bottom) / 2.0
        self.top = centro + (max_depth * self.row) / 2.0
        return self

    def _layout(self, snaps: list[dict]) -> dict[int, np.ndarray]:
        if not snaps:
            return {}
        cols = max(s["order"] for s in snaps) + 1
        gap = min(self.span / max(cols - 1, 1), self.max_col)
        mid = (cols - 1) / 2.0
        return {s["id"]: np.array([(s["order"] - mid) * gap,
                                   self.top - s["depth"] * self.row, 0.0])
                for s in snaps}

    @staticmethod
    def _colors(color: str) -> tuple[str, str]:
        return (RED_FILL, RED_STROKE) if color == "R" else (BLK_FILL, BLK_STROKE)

    def _make_node(self, snap: dict, pos: np.ndarray) -> VGroup:
        fill, stroke = self._colors(snap["color"])
        circle = Circle(radius=self.radius, fill_color=fill, fill_opacity=1.0,
                        stroke_color=stroke, stroke_width=3.0)
        label = Text(str(snap["value"]), font_size=20, color=TXT, weight=BOLD)
        if label.width > self.radius * 1.45:
            label.scale_to_fit_width(self.radius * 1.45)
        group = VGroup(circle, label).move_to(pos)
        label.move_to(circle.get_center())
        return group

    def _build_edges(self) -> VGroup:
        """Se re-evalua cada frame: las aristas siguen a los nodos que se mueven."""
        group = VGroup()
        for parent, child in self.links:
            a, b = self.nodes.get(parent), self.nodes.get(child)
            if a is None or b is None:
                continue
            opacity = float(min(a[0].get_fill_opacity(), b[0].get_fill_opacity()))
            if opacity <= 0.03:
                continue
            pa, pb = a.get_center(), b.get_center()
            d = pb - pa
            norma = float(np.linalg.norm(d))
            if norma < 1e-6:
                continue
            u = d / norma
            group.add(Line(pa + u * self.radius, pb - u * self.radius,
                           stroke_color=EDGE, stroke_width=3.2,
                           stroke_opacity=opacity * 0.9))
        return group

    @staticmethod
    def _links_of(snaps: list[dict]) -> list[tuple[int, int]]:
        out = []
        for s in snaps:
            for child in (s["left"], s["right"]):
                if child != -1:
                    out.append((s["id"], child))
        return out

    def update_to(self, event: dict) -> list[Animation]:
        snaps = event["nodes"]
        target = self._layout(snaps)
        by_id = {s["id"]: s for s in snaps}
        anims: list[Animation] = []

        for nid in list(self.nodes):
            if nid not in by_id:
                anims.append(FadeOut(self.nodes[nid], scale=0.4))
                self.node_layer.remove(self.nodes[nid])
                del self.nodes[nid]

        # La topologia cambia de golpe; las posiciones se interpolan.
        self.links = self._links_of(snaps)

        for nid, snap in by_id.items():
            pos = target[nid]
            if nid in self.nodes:
                node = self.nodes[nid]
                fill, stroke = self._colors(snap["color"])
                moved = float(np.linalg.norm(node.get_center() - pos)) > 1e-3
                recolored = node[0].get_fill_color().to_hex().upper() != fill.upper()
                if moved or recolored:
                    ghost = node.copy().move_to(pos)
                    ghost[0].set_fill(fill).set_stroke(stroke)
                    ghost[1].move_to(pos)
                    # En una rotacion varios nodos cambian de lado a la vez. Si
                    # todos van en linea recta se cruzan unos con otros y se ve
                    # como un amontonamiento. Con un arco comun describen curvas
                    # paralelas y se leen como un giro.
                    dx = float(pos[0] - node.get_center()[0])
                    arc = 0.45 if abs(dx) > 0.15 else 0.0
                    anims.append(Transform(node, ghost, path_arc=arc))
            else:
                node = self._make_node(snap, pos)   # sin set_opacity: FadeIn va 0 -> 1
                self.nodes[nid] = node
                self.node_layer.add(node)
                anims.append(FadeIn(node, scale=0.5))
        return anims

    def ring(self, ids: list[int], color: str) -> VGroup:
        group = VGroup()
        for nid in ids:
            node = self.nodes.get(nid)
            if node is not None:
                group.add(Circle(radius=self.radius + 0.11, stroke_color=color,
                                 stroke_width=4.5, fill_opacity=0.0)
                          .move_to(node.get_center()))
        return group

    def reset(self, scene: Scene, run_time: float = 0.4):
        olds = list(self.node_layer.submobjects)
        if olds:
            scene.play(*[FadeOut(m, scale=0.5) for m in olds], run_time=run_time)
        if olds:
            self.node_layer.remove(*olds)
        self.nodes.clear()
        self.links.clear()
        scene.add(self.edge_layer, self.node_layer)   # reafirma el orden z


# --------------------------------------------------------------------------
# Escena base
# --------------------------------------------------------------------------
class RBScene(Scene):
    PART = ""
    PART_TITLE = ""

    def setup(self):
        self.camera.background_color = BG

    def wait(self, duration: float = 1.0, **kwargs):
        """Todas las pausas de lectura pasan por aqui, escaladas por TEMPO_WAIT."""
        return super().wait(duration * TEMPO_WAIT, **kwargs)

    def build_stage(self, subtitle: str = " "):
        hdr = Text(self.PART_TITLE, font_size=27, color=TXT, weight=BOLD)
        hdr.to_corner(UL, buff=0.40)
        badge = Text(f"Parte {self.PART}", font_size=19, color=MUTED)
        badge.to_corner(UR, buff=0.40)
        self.rule = Line(LEFT * 6.9, RIGHT * 6.9, stroke_color=EDGE,
                         stroke_width=1.4, stroke_opacity=0.5)
        self.rule.next_to(hdr, DOWN, buff=0.20).set_x(0)

        self.h_caption = VGroup()
        self.h_status = VGroup()
        self.h_case = VGroup()
        self.h_rings = VGroup()
        self.add(hdr, badge, self.rule,
                 self.h_caption, self.h_status, self.h_case, self.h_rings)
        self.set_caption(subtitle)

    def set_caption(self, text: str):
        t = Text(wrap(text), font_size=22, color=TXT, line_spacing=0.75)
        fit_width(t)
        t.to_edge(DOWN, buff=0.52)
        swap(self.h_caption, t)

    def set_status(self, black_height: int, valid: bool):
        label = "propiedades RB: OK" if valid else "propiedades RB: reparando"
        t = Text(f"altura negra = {black_height}    |    {label}",
                 font_size=19, color=OK if valid else WARN)
        t.next_to(self.rule, DOWN, buff=0.16).to_edge(RIGHT, buff=0.5)
        swap(self.h_status, t)

    def clear_status(self):
        """Oculta el panel de altura negra en las secciones sin arbol en pantalla."""
        swap(self.h_status)

    def set_case(self, case: str):
        if not case:
            swap(self.h_case)
            return
        label = Text(case, font_size=20, color=BG, weight=BOLD)
        box = RoundedRectangle(corner_radius=0.12, width=label.width + 0.50,
                               height=label.height + 0.26, fill_color=ACCENT,
                               fill_opacity=1.0, stroke_width=0)
        group = VGroup(box, label)
        group.next_to(self.rule, DOWN, buff=0.16).to_edge(LEFT, buff=0.5)
        swap(self.h_case, group)

    def play_trace(self, tree: TreeView, events: list[dict], *,
                   speed: float = 1.0, skip: set[str] | None = None,
                   show_status: bool = True, show_case: bool = True):
        skip = skip or set()
        for ev in events:
            action = ev["action"]
            if action in skip:
                continue

            anims = tree.update_to(ev)

            if ev["detail"]:
                self.set_caption(ev["detail"])
            if show_status:
                self.set_status(ev["black_height"], ev["valid"])
            if show_case:
                self.set_case(ev.get("case", ""))

            rings = VGroup()
            if action in FOCUS_ACTIONS:
                rings.add(tree.ring(ev.get("focus", []), ACCENT))
                rings.add(tree.ring(ev.get("secondary", []), ACCENT2))
            swap(self.h_rings, rings)

            # Todo lo de play_trace lo gobierna TEMPO_TRACE. Se llama a
            # Scene.wait directamente para saltarse el override de la escena,
            # que aplicaria TEMPO_WAIT encima y escalaria dos veces.
            run_time = RUN_TIME.get(action, 0.5) * speed * TEMPO_TRACE
            if anims:
                self.play(*anims, run_time=run_time)
            else:
                Scene.wait(self, run_time)
            hold = HOLD.get(action, 0.0) * speed * TEMPO_TRACE
            if hold:
                Scene.wait(self, hold)

        swap(self.h_rings)
        self.set_case("")

    def bullets(self, lines: list[str], font_size: int = 25, buff: float = 0.32,
                dot_color: str = ACCENT) -> VGroup:
        group = VGroup()
        for line in lines:
            dot = Dot(radius=0.055, color=dot_color)
            body = Text(line, font_size=font_size, color=TXT)
            group.add(VGroup(dot, body).arrange(RIGHT, buff=0.24))
        group.arrange(DOWN, buff=buff, aligned_edge=LEFT)
        return group
