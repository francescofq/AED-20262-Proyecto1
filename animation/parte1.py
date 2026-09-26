"""
parte1.py - PARTE 1/3: Introducción conceptual, TDA y propiedades.

Renderizar:
  python -m manim render -qh parte1.py Parte1
"""
from manim import *

from config_video import CURSO, PERIODO, TITULO, SUBTITULO, INTEGRANTES
from rbviz import (ACCENT, ACCENT2, BLK_FILL, BLK_STROKE, EDGE, MUTED, OK,
                   RED_FILL, RED_STROKE, TXT, RBScene, TreeView, events_of,
                   grid)


class Parte1(RBScene):
    PART = "1/3"
    PART_TITLE = "Qué es un Red-Black Tree"

    def construct(self):
        self.caratula()
        self.build_stage("Un BST que se mantiene balanceado usando un bit de color por nodo.")
        self.que_es()
        self.el_tda()
        self.propiedades()
        self.construccion_real()
        self.donde_se_usa()
        self.cierre()

    # ------------------------------------------------------------------
    def caratula(self):
        rojo = Circle(radius=0.30, fill_color=RED_FILL, fill_opacity=1,
                      stroke_color=RED_STROKE, stroke_width=3)
        negro = Circle(radius=0.30, fill_color=BLK_FILL, fill_opacity=1,
                       stroke_color=BLK_STROKE, stroke_width=3)
        emblema = VGroup(negro, rojo).arrange(RIGHT, buff=0.22)

        titulo = Text(TITULO, font_size=62, color=TXT, weight=BOLD)
        subtitulo = Text(SUBTITULO, font_size=27, color=MUTED)
        linea = Line(LEFT * 3.2, RIGHT * 3.2, stroke_color=EDGE, stroke_width=1.6)
        integrantes = VGroup(*[Text(n, font_size=23, color=TXT) for n in INTEGRANTES])
        integrantes.arrange(DOWN, buff=0.20)
        curso = Text(f"{CURSO}   |   {PERIODO}", font_size=19, color=MUTED)

        bloque = VGroup(emblema, titulo, subtitulo, linea, integrantes, curso)
        bloque.arrange(DOWN, buff=0.40).move_to(ORIGIN)

        self.play(FadeIn(emblema, scale=0.6), run_time=0.6)
        self.play(Write(titulo), run_time=0.9)
        self.play(FadeIn(subtitulo, shift=UP * 0.2), run_time=0.5)
        self.play(Create(linea), run_time=0.4)
        self.play(LaggedStart(*[FadeIn(t, shift=UP * 0.15) for t in integrantes],
                              lag_ratio=0.25), run_time=0.9)
        self.play(FadeIn(curso), run_time=0.4)
        self.wait(1.4)
        self.play(FadeOut(bloque), run_time=0.6)

    # ------------------------------------------------------------------
    def que_es(self):
        idea = self.bullets([
            "Es un BST: a la izquierda los menores, a la derecha los mayores.",
            "Cada nodo guarda un bit extra de color: ROJO o NEGRO.",
            "Los colores no son parte de los datos. Sirven para balancear.",
            "La altura se mantiene en O(log n) sin importar el orden de entrada.",
        ], font_size=26, buff=0.46)
        idea.move_to(DOWN * 0.15)
        self.play(LaggedStart(*[FadeIn(b, shift=RIGHT * 0.3) for b in idea],
                              lag_ratio=0.35), run_time=2.1)
        self.wait(2.0)
        self.play(FadeOut(idea), run_time=0.5)

    # ------------------------------------------------------------------
    def el_tda(self):
        self.clear_status()
        self.set_caption("El TDA que implementa es un Conjunto Ordenado.")
        titulo = Text("TDA: Conjunto Ordenado (Ordered Set / Map)",
                      font_size=32, color=TXT, weight=BOLD)

        ops = [("insert(x)", "O(log n)"),
               ("remove(x)", "O(log n)"),
               ("contains(x)", "O(log n)"),
               ("min() / max()", "O(log n)"),
               ("recorrido in-order", "O(n), ya ordenado")]
        tabla = grid([[Text(op, font_size=25, color=TXT),
                       Text(costo, font_size=23, color=ACCENT2)]
                      for op, costo in ops],
                     col_buff=1.10, row_buff=0.30)
        tabla.next_to(titulo, DOWN, buff=0.55)

        nota = Text("El TDA no habla de colores. Son parte de la implementación.",
                    font_size=21, color=MUTED)
        nota.next_to(tabla, DOWN, buff=0.50)
        VGroup(titulo, tabla, nota).move_to(DOWN * 0.15)

        self.play(FadeIn(titulo, shift=DOWN * 0.2), run_time=0.6)
        self.play(LaggedStart(*[FadeIn(f, shift=RIGHT * 0.25) for f in tabla],
                              lag_ratio=0.22), run_time=1.6)
        self.play(FadeIn(nota), run_time=0.5)
        self.wait(2.2)
        self.play(FadeOut(VGroup(titulo, tabla, nota)), run_time=0.5)

    # ------------------------------------------------------------------
    def propiedades(self):
        self.clear_status()
        self.set_caption("El árbol solo tiene que mantener estas 5 reglas.")
        titulo = Text("Las 5 propiedades", font_size=32, color=TXT, weight=BOLD)

        props = self.bullets([
            "1.  Todo nodo es ROJO o NEGRO.",
            "2.  La raíz es NEGRA.",
            "3.  Toda hoja nil es NEGRA.",
            "4.  Un nodo ROJO no puede tener hijos ROJOS.",
            "5.  Todo camino raíz → nil tiene la misma cantidad de NEGROS.",
        ], font_size=26, buff=0.34)
        props.next_to(titulo, DOWN, buff=0.55)

        clave = Text("(4) + (5)   ⇒   altura ≤ 2 · log₂(n + 1)",
                     font_size=26, color=OK, weight=BOLD)
        clave.next_to(props, DOWN, buff=0.55)
        VGroup(titulo, props, clave).move_to(DOWN * 0.15)

        self.play(FadeIn(titulo, shift=DOWN * 0.2), run_time=0.5)
        self.play(LaggedStart(*[FadeIn(b, shift=RIGHT * 0.25) for b in props],
                              lag_ratio=0.28), run_time=2.2)
        self.wait(1.4)
        self.play(FadeIn(clave, scale=0.9), run_time=0.7)
        self.wait(2.2)
        self.play(FadeOut(VGroup(titulo, props, clave)), run_time=0.5)

    # ------------------------------------------------------------------
    def construccion_real(self):
        self.set_caption("Construcción paso a paso, con la implementación en C++ detrás.")
        rotulo = Text("Insertando 41, 38, 31, 12, 19, 8", font_size=24, color=MUTED)
        rotulo.move_to(DOWN * 2.50)
        self.play(FadeIn(rotulo), run_time=0.4)

        evs = events_of("p1_construccion")
        tree = TreeView(self).fit(evs)
        self.play_trace(tree, evs, speed=0.70, skip={"compare"})

        cierre = Text("El árbol se reacomodó solo en cada inserción.",
                      font_size=25, color=OK)
        cierre.move_to(DOWN * 2.50)
        self.play(FadeOut(rotulo), FadeIn(cierre), run_time=0.5)
        self.wait(1.6)
        self.play(FadeOut(cierre), run_time=0.4)
        tree.reset(self)

    # ------------------------------------------------------------------
    def donde_se_usa(self):
        self.clear_status()
        self.set_caption("Se usa en software real, no solo en clase.")
        titulo = Text("Dónde se usa", font_size=32, color=TXT, weight=BOLD)

        usos = self.bullets([
            "std::map y std::set de C++ (típicamente un RB-Tree).",
            "java.util.TreeMap y TreeSet.",
            "Planificador CFS del kernel de Linux.",
            "Índices de sistemas de archivos y bases de datos.",
        ], font_size=25, buff=0.38, dot_color=ACCENT2)
        usos.next_to(titulo, DOWN, buff=0.60)
        VGroup(titulo, usos).move_to(DOWN * 0.15)

        self.play(FadeIn(titulo, shift=DOWN * 0.2), run_time=0.5)
        self.play(LaggedStart(*[FadeIn(b, shift=RIGHT * 0.25) for b in usos],
                              lag_ratio=0.3), run_time=1.8)
        self.wait(2.2)
        self.play(FadeOut(VGroup(titulo, usos)), run_time=0.5)

    # ------------------------------------------------------------------
    def cierre(self):
        self.clear_status()
        self.set_caption(" ")
        siguiente = Text("Parte 2: cómo se mantiene el balance al insertar",
                         font_size=30, color=TXT, weight=BOLD)
        siguiente.move_to(ORIGIN)
        self.play(FadeIn(siguiente, scale=0.92), run_time=0.7)
        self.wait(1.3)
        self.play(FadeOut(siguiente), run_time=0.5)
