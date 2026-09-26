"""
parte3.py - PARTE 3/3: Casos borde, búsqueda, eliminación y complejidad.

Renderizar:
  python -m manim render -qh parte3.py Parte3
"""
from manim import *

from config_video import CURSO, INTEGRANTES, PERIODO, TITULO
from rbviz import (ACCENT, ACCENT2, EDGE, MUTED, OK, TXT, WARN, RBScene,
                   TreeView, events_of, fit_width, grid, load_stats)


class Parte3(RBScene):
    PART = "3/3"
    PART_TITLE = "Casos borde y complejidad"

    def construct(self):
        self.portada()
        self.build_stage("Ahora los casos borde: dónde suelen romperse los árboles.")
        self.borde_arbol_vacio()
        self.borde_peor_caso()
        self.comparacion_real()
        self.busqueda()
        self.eliminacion()
        self.complejidad()
        self.conclusiones()
        self.creditos()

    # ------------------------------------------------------------------
    def portada(self):
        t = Text("Parte 3 — Casos borde y complejidad", font_size=42,
                 color=TXT, weight=BOLD)
        s = Text("Árbol vacío, peor caso, eliminación y costos", font_size=25,
                 color=MUTED)
        g = VGroup(t, s).arrange(DOWN, buff=0.36)
        self.play(FadeIn(t, shift=UP * 0.2), run_time=0.55)
        self.play(FadeIn(s), run_time=0.35)
        self.wait(1.0)
        self.play(FadeOut(g), run_time=0.4)

    def _rotulo(self, texto: str, color: str = MUTED) -> Text:
        return fit_width(
            Text(texto, font_size=24, color=color)).move_to(DOWN * 2.45)

    # ------------------------------------------------------------------
    def borde_arbol_vacio(self):
        self.set_caption("Caso borde 1: insertar en un árbol vacío.")
        rot = self._rotulo("Caso borde — árbol vacío")
        self.play(FadeIn(rot), run_time=0.35)

        evs = events_of("p3_borde_vacio")
        tree = TreeView(self).fit(evs)
        self.play_trace(tree, evs, speed=1.15)

        fin = self._rotulo("Entra ROJO, pero la propiedad 2 obliga a pintarlo de NEGRO.", OK)
        self.play(FadeOut(rot), FadeIn(fin), run_time=0.45)
        self.wait(1.8)
        self.play(FadeOut(fin), run_time=0.35)
        tree.reset(self)

    # ------------------------------------------------------------------
    def borde_peor_caso(self):
        self.set_caption("Caso borde 2: el peor caso de un BST, insertar ya ordenado.")
        rot = self._rotulo("Insertando 1, 2, 3, ... 10 en orden creciente")
        self.play(FadeIn(rot), run_time=0.35)

        evs = events_of("p3_borde_ordenado")
        tree = TreeView(self).fit(evs)
        self.play_trace(tree, evs, speed=0.42, skip={"compare"})

        fin = self._rotulo("Un BST normal habría quedado en 10 niveles. Este quedó en 5.", OK)
        self.play(FadeOut(rot), FadeIn(fin), run_time=0.45)
        self.wait(1.9)
        self.play(FadeOut(fin), run_time=0.35)
        tree.reset(self)

    # ------------------------------------------------------------------
    def comparacion_real(self):
        """Tabla con alturas MEDIDAS por el programa en C++ (traces/p3_stats.json)."""
        self.clear_status()
        self.set_caption("Alturas medidas por nuestro programa.")
        titulo = Text("Altura con n claves insertadas en orden", font_size=30,
                      color=TXT, weight=BOLD)

        stats = load_stats()
        filas = [[Text("n", font_size=22, color=TXT, weight=BOLD),
                  Text("BST sin balanceo", font_size=22, color=WARN, weight=BOLD),
                  Text("Red-Black Tree", font_size=22, color=OK, weight=BOLD),
                  Text("cota 2·log₂(n+1)", font_size=22, color=MUTED, weight=BOLD)]]
        for row in stats:
            filas.append([
                Text(f"{row['n']:,}".replace(",", " "), font_size=22, color=TXT),
                Text(f"{row['altura_bst']:,}".replace(",", " "), font_size=22, color=WARN),
                Text(str(row["altura_rbt"]), font_size=22, color=OK),
                Text(str(row["cota_2log2"]), font_size=22, color=MUTED),
            ])
        tabla = grid(filas, col_buff=0.95, row_buff=0.32,
                     aligns=[RIGHT, RIGHT, RIGHT, RIGHT])
        tabla.next_to(titulo, DOWN, buff=0.50)

        sep = Line(LEFT * (tabla.width / 2 + 0.2), RIGHT * (tabla.width / 2 + 0.2),
                   stroke_color=EDGE, stroke_width=1.2)
        sep.move_to([tabla.get_x(), (tabla[0].get_bottom()[1] + tabla[1].get_top()[1]) / 2, 0])

        remate = Text("Con 100 000 claves ordenadas: 100 000 niveles contra 31.",
                      font_size=22, color=OK)
        remate.next_to(tabla, DOWN, buff=0.50)
        VGroup(titulo, tabla, sep, remate).move_to(DOWN * 0.15)

        self.play(FadeIn(titulo, shift=DOWN * 0.2), run_time=0.5)
        self.play(FadeIn(tabla[0]), Create(sep), run_time=0.5)
        self.play(LaggedStart(*[FadeIn(f, shift=RIGHT * 0.2) for f in tabla[1:]],
                              lag_ratio=0.25), run_time=1.5)
        self.play(FadeIn(remate), run_time=0.5)
        self.wait(3.0)
        self.play(FadeOut(VGroup(titulo, tabla, sep, remate)), run_time=0.5)

    # ------------------------------------------------------------------
    def busqueda(self):
        self.set_caption("La búsqueda es igual que en un BST normal.")
        rot = self._rotulo("Buscando 19 (existe) y 99 (no existe)")
        self.play(FadeIn(rot), run_time=0.35)

        evs = events_of("p3_busqueda")
        tree = TreeView(self).fit(evs)
        self.play_trace(tree, evs, speed=1.0)

        fin = self._rotulo("Cada comparación descarta la mitad del árbol.", OK)
        self.play(FadeOut(rot), FadeIn(fin), run_time=0.45)
        self.wait(1.7)
        self.play(FadeOut(fin), run_time=0.35)
        tree.reset(self)

    # ------------------------------------------------------------------
    def eliminacion(self):
        self.set_caption("Eliminar es más difícil: quitar un nodo NEGRO rompe la propiedad 5.")
        rot = self._rotulo("Eliminando 40 y luego 20")
        self.play(FadeIn(rot), run_time=0.35)

        evs = events_of("p3_eliminacion")
        tree = TreeView(self).fit(evs)
        self.play_trace(tree, evs, speed=0.95)

        fin = self._rotulo("En estas dos eliminaciones se dispararon los 4 casos de fix_delete.", OK)
        self.play(FadeOut(rot), FadeIn(fin), run_time=0.45)
        self.wait(1.9)
        self.play(FadeOut(fin), run_time=0.35)
        tree.reset(self)

    # ------------------------------------------------------------------
    def complejidad(self):
        self.clear_status()
        self.set_caption(" ")
        titulo = Text("Complejidad temporal", font_size=33, color=TXT, weight=BOLD)

        filas = [
            ("búsqueda", "O(log n)", "baja un nivel por comparación"),
            ("inserción", "O(log n)", "≤ 2 rotaciones + O(log n) recoloreos"),
            ("eliminación", "O(log n)", "≤ 3 rotaciones + O(log n) recoloreos"),
            ("espacio", "O(n)", "1 bit de color por nodo"),
        ]
        tabla = grid([[Text(op, font_size=25, color=TXT),
                       Text(costo, font_size=25, color=OK, weight=BOLD),
                       Text(nota, font_size=21, color=MUTED)]
                      for op, costo, nota in filas],
                     col_buff=0.80, row_buff=0.42)
        tabla.next_to(titulo, DOWN, buff=0.60)

        cota = Text("Todo sale de una sola cota:   altura ≤ 2 · log₂(n + 1)",
                    font_size=26, color=ACCENT, weight=BOLD)
        cota.next_to(tabla, DOWN, buff=0.60)
        VGroup(titulo, tabla, cota).move_to(DOWN * 0.15)

        self.play(FadeIn(titulo, shift=DOWN * 0.2), run_time=0.5)
        self.play(LaggedStart(*[FadeIn(f, shift=RIGHT * 0.25) for f in tabla],
                              lag_ratio=0.25), run_time=1.5)
        self.play(FadeIn(cota, scale=0.92), run_time=0.6)
        self.wait(2.8)
        self.play(FadeOut(VGroup(titulo, tabla, cota)), run_time=0.5)

    # ------------------------------------------------------------------
    def conclusiones(self):
        self.clear_status()
        titulo = Text("Conclusiones", font_size=33, color=TXT, weight=BOLD)
        ideas = self.bullets([
            "Con un solo bit de color por nodo, el árbol garantiza altura\nO(log n) sin importar en qué orden lleguen los datos.",
            "Balancear sale barato: como máximo 2 rotaciones al insertar\ny 3 al eliminar. El resto son recoloreos.",
        ], font_size=24, buff=0.55)
        ideas.next_to(titulo, DOWN, buff=0.70)
        VGroup(titulo, ideas).move_to(DOWN * 0.15)

        self.play(FadeIn(titulo, shift=DOWN * 0.2), run_time=0.5)
        self.play(LaggedStart(*[FadeIn(b, shift=RIGHT * 0.25) for b in ideas],
                              lag_ratio=0.4), run_time=1.4)
        self.wait(3.2)
        self.play(FadeOut(VGroup(titulo, ideas)), run_time=0.5)

    # ------------------------------------------------------------------
    def creditos(self):
        self.set_caption(" ")
        # self.mobjects puede contener mobjects que no son VMobject
        # (p. ej. el always_redraw de las aristas), asi que no se agrupan.
        if self.mobjects:
            self.play(*[FadeOut(m) for m in list(self.mobjects)], run_time=0.5)

        titulo = Text(TITULO, font_size=48, color=TXT, weight=BOLD)
        linea = Line(LEFT * 3.0, RIGHT * 3.0, stroke_color=EDGE, stroke_width=1.6)
        nombres = VGroup(*[Text(n, font_size=23, color=TXT) for n in INTEGRANTES])
        nombres.arrange(DOWN, buff=0.20)
        curso = Text(f"{CURSO}   |   {PERIODO}", font_size=19, color=MUTED)
        nota = Text("Animación hecha con Manim a partir de nuestra implementación en C++",
                    font_size=18, color=MUTED)

        bloque = VGroup(titulo, linea, nombres, curso, nota)
        bloque.arrange(DOWN, buff=0.40).move_to(ORIGIN)

        self.play(FadeIn(titulo, scale=0.94), run_time=0.7)
        self.play(Create(linea), run_time=0.35)
        self.play(LaggedStart(*[FadeIn(t, shift=UP * 0.15) for t in nombres],
                              lag_ratio=0.22), run_time=0.9)
        self.play(FadeIn(curso), FadeIn(nota), run_time=0.6)
        self.wait(2.6)
        self.play(FadeOut(bloque), run_time=0.8)
