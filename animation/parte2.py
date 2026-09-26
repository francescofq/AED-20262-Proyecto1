"""
parte2.py - PARTE 2/3: Inserción y los tres casos de rebalanceo.

Renderizar:
  python -m manim render -qh parte2.py Parte2
"""
from collections import Counter

from manim import *

from rbviz import (ACCENT, ACCENT2, BLK_FILL, BLK_STROKE, MUTED, OK, RED_FILL,
                   RED_STROKE, TXT, RBScene, TreeView, events_of, fit_width, grid)


class Parte2(RBScene):
    PART = "2/3"
    PART_TITLE = "Inserción y rebalanceo"

    def construct(self):
        self.portada()
        self.build_stage("Insertar es lo fácil. Lo difícil es mantener las 5 propiedades.")
        self.por_que_rojo()
        self.caso_1()
        self.caso_3()
        self.caso_2()
        self.secuencia()
        self.resumen()

    # ------------------------------------------------------------------
    def portada(self):
        t = Text("Parte 2 — Inserción y rebalanceo", font_size=44,
                 color=TXT, weight=BOLD)
        s = Text("Los tres casos de fix_insert", font_size=26, color=MUTED)
        g = VGroup(t, s).arrange(DOWN, buff=0.38)
        self.play(FadeIn(t, shift=UP * 0.2), run_time=0.6)
        self.play(FadeIn(s), run_time=0.4)
        self.wait(1.1)
        self.play(FadeOut(g), run_time=0.45)

    # ------------------------------------------------------------------
    def por_que_rojo(self):
        self.clear_status()
        titulo = Text("¿Por qué el nodo nuevo entra ROJO?", font_size=33,
                      color=TXT, weight=BOLD)

        rojo = Circle(radius=0.34, fill_color=RED_FILL, fill_opacity=1,
                      stroke_color=RED_STROKE, stroke_width=3)
        negro = Circle(radius=0.34, fill_color=BLK_FILL, fill_opacity=1,
                       stroke_color=BLK_STROKE, stroke_width=3)

        col_r = VGroup(rojo, Text("entra ROJO", font_size=21, color=TXT),
                       Text("como mucho rompe\nla propiedad 4", font_size=19,
                            color=OK, line_spacing=0.7))
        col_r.arrange(DOWN, buff=0.26)
        col_n = VGroup(negro, Text("entra NEGRO", font_size=21, color=TXT),
                       Text("cambia la altura negra\nde todo un subárbol",
                            font_size=19, color=ACCENT, line_spacing=0.7))
        col_n.arrange(DOWN, buff=0.26)

        par = VGroup(col_r, col_n).arrange(RIGHT, buff=2.4)
        par.next_to(titulo, DOWN, buff=0.70)

        remate = Text("Un rojo mal puesto se arregla cerca. Un negro obliga a revisar mucho más.",
                      font_size=22, color=MUTED)
        remate.next_to(par, DOWN, buff=0.60)
        VGroup(titulo, par, remate).move_to(DOWN * 0.15)

        self.play(FadeIn(titulo, shift=DOWN * 0.2), run_time=0.5)
        self.play(FadeIn(col_r, shift=RIGHT * 0.3), run_time=0.6)
        self.play(FadeIn(col_n, shift=LEFT * 0.3), run_time=0.6)
        self.play(FadeIn(remate), run_time=0.5)
        self.wait(2.4)
        self.play(FadeOut(VGroup(titulo, par, remate)), run_time=0.5)

    # ------------------------------------------------------------------
    def _rotulo(self, texto: str, color: str = MUTED) -> Text:
        return fit_width(
            Text(texto, font_size=24, color=color)).move_to(DOWN * 2.50)

    def _caso(self, trace: str, rotulo: str, cierre: str, speed: float = 1.00):
        evs = events_of(trace)
        rot = self._rotulo(rotulo)
        self.play(FadeIn(rot), run_time=0.35)

        tree = TreeView(self).fit(evs)
        self.play_trace(tree, evs, speed=speed)

        fin = self._rotulo(cierre, OK)
        self.play(FadeOut(rot), FadeIn(fin), run_time=0.45)
        self.wait(1.4)
        self.play(FadeOut(fin), run_time=0.35)
        tree.reset(self)

    def caso_1(self):
        self.set_caption("Caso 1: el tío también es ROJO.")
        self._caso(
            "p2_caso1_tio_rojo",
            "Caso 1 — insertando 10, 5, 15, 1",
            "No hubo rotaciones: solo se repintaron padre, tío y abuelo.",
        )

    def caso_3(self):
        self.set_caption("Caso 3: el tío es NEGRO y el nodo está en línea con padre y abuelo.")
        self._caso(
            "p2_caso3_zigzig",
            "Caso 3 (zig-zig) — insertando 10, 20, 30",
            "Con una rotación sobre el abuelo queda balanceado.",
        )

    def caso_2(self):
        self.set_caption("Caso 2: el tío es NEGRO pero el nodo está en zig-zag.")
        self._caso(
            "p2_caso2_zigzag",
            "Caso 2 (zig-zag) — insertando 10, 5, 7",
            "El Caso 2 no termina el trabajo: alinea el nodo y pasa al Caso 3.",
        )

    # ------------------------------------------------------------------
    def secuencia(self):
        """Corrida larga: los tres casos encadenandose sobre un arbol que crece."""
        evs = events_of("p2_secuencia")

        # El recuento de casos se lee del trace, no se escribe a mano: si se
        # cambia la secuencia en main.cpp, este texto sigue siendo cierto.
        conteo = Counter(e["case"] for e in evs if e.get("case"))
        resumen = ", ".join(
            f"Caso {n} ×{conteo[c]}"
            for n, c in ((1, "Caso 1: tío ROJO"),
                         (2, "Caso 2: tío NEGRO, zig-zag"),
                         (3, "Caso 3: tío NEGRO, zig-zig"))
            if conteo.get(c))

        self.set_caption("Ahora los tres casos juntos, sobre un árbol que va creciendo.")
        rot = self._rotulo("Insertando 7, 3, 18, 10, 22, 8, 11, 26")
        self.play(FadeIn(rot), run_time=0.35)

        tree = TreeView(self).fit(evs)
        self.play_trace(tree, evs, speed=0.68, skip={"compare"})

        fin = self._rotulo(f"En esta corrida: {resumen}.", OK)
        self.play(FadeOut(rot), FadeIn(fin), run_time=0.45)
        self.wait(2.2)
        self.play(FadeOut(fin), run_time=0.35)
        tree.reset(self)

    # ------------------------------------------------------------------
    def resumen(self):
        self.clear_status()
        self.set_caption(" ")
        titulo = Text("Resumen de los tres casos", font_size=31,
                      color=TXT, weight=BOLD)

        filas = [
            ("Caso 1", "tío ROJO", "recolorear y subir al abuelo", ACCENT2),
            ("Caso 2", "tío NEGRO, zig-zag", "rotar el padre → pasa a Caso 3", ACCENT),
            ("Caso 3", "tío NEGRO, en línea", "recolorear y rotar el abuelo: fin", OK),
        ]
        tabla = grid([[Text(n, font_size=25, color=c, weight=BOLD),
                       Text(cond, font_size=23, color=TXT),
                       Text(acc, font_size=23, color=MUTED)]
                      for n, cond, acc, c in filas],
                     col_buff=0.70, row_buff=0.42)
        tabla.next_to(titulo, DOWN, buff=0.70)

        remate = VGroup(
            Text("El Caso 1 puede repetirse subiendo por el árbol: O(log n) recoloreos.",
                 font_size=22, color=TXT),
            Text("Los Casos 2 y 3 cierran el bucle: como máximo 2 rotaciones por inserción.",
                 font_size=22, color=OK),
        ).arrange(DOWN, buff=0.24)
        remate.next_to(tabla, DOWN, buff=0.65)
        VGroup(titulo, tabla, remate).move_to(DOWN * 0.15)

        self.play(FadeIn(titulo, shift=DOWN * 0.2), run_time=0.5)
        self.play(LaggedStart(*[FadeIn(f, shift=RIGHT * 0.25) for f in tabla],
                              lag_ratio=0.3), run_time=1.5)
        self.play(FadeIn(remate), run_time=0.6)
        self.wait(2.8)
        self.play(FadeOut(VGroup(titulo, tabla, remate)), run_time=0.5)

        siguiente = Text("Parte 3: casos borde, eliminación y complejidad",
                         font_size=30, color=TXT, weight=BOLD)
        self.play(FadeIn(siguiente, scale=0.92), run_time=0.7)
        self.wait(1.3)
        self.play(FadeOut(siguiente), run_time=0.5)
