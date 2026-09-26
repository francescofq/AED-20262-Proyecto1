# Red-Black Tree — Anima tu Estructura de Datos

Proyecto Final 1 · **CS2023 Algoritmos y Estructuras de Datos** · 2026-2
Prof. Víctor Racsó Galván Oyola

Video educativo que explica y demuestra visualmente el funcionamiento de un
**Red-Black Tree**: un Árbol Binario de Búsqueda auto-balanceado mediante
coloreado de nodos y rotaciones.

> **Integrantes:** Jose Ruiz Lam (202510050) · Osmar Vilchez Aguirre (202510122) ·
> Francesco Ferrante Quino (202510174)

---

## La animación no está dibujada a mano

El requisito del proyecto es que la animación esté impulsada por una
implementación real. Aquí eso es literal:

```
main.cpp + rbtree.hpp          traces/*.json              animation/parte*.py
  implementación propia   ──▶   eventos + snapshots   ──▶   Manim los dibuja
       (C++20)                     del árbol real            (solo dibuja)
```

* La estructura está implementada desde cero en C++ (variante CLRS con nodo
  centinela `nil`). No se usa `std::map`, `std::set` ni ninguna librería que
  ya provea un árbol balanceado.
* `rbtree.hpp` conserva el algoritmo original del grupo sin cambios:
  `rotate_left`, `rotate_right`, `fix_delete`, `transplant`, `search`,
  `minimum`, `contains` y `print_helper` son idénticos línea por línea al
  primer commit. Lo único añadido son las llamadas al tracer, un `id` por
  nodo (necesario para seguir un nodo entre fotogramas) y `validate()`.
* Cada paso atómico del algoritmo —cada comparación, recoloreo, rotación y
  caso de rebalanceo— emite un evento con un **snapshot completo del árbol**
  tal como está en memoria en ese instante.
* Incluso la **posición en pantalla** de cada nodo sale del árbol real: los
  campos `depth` y `order` de cada snapshot se calculan recorriendo la
  estructura, no los inventa el animador.
* El código incluye `validate()`, que verifica las 5 propiedades del RB-Tree.
  Su resultado se muestra en pantalla durante toda la animación: se ve el
  invariante romperse durante un `fixup` y volver a cumplirse al terminar.

Si `traces/` está vacío, la animación falla con un error explícito: no tiene
datos propios de dónde dibujar.

---

## Software requerido

| Herramienta | Versión usada | Para qué |
|---|---|---|
| `g++` (o `clang++`) con C++20 | g++ 15.2 | compilar la estructura y generar los traces |
| CMake *(opcional)* | 3.20+ | build alternativo al `g++` directo |
| Python | **3.12** | ejecutar Manim |
| [Manim Community](https://docs.manim.community/) | 0.21.0 | renderizar la animación |
| `imageio-ffmpeg` | 0.5+ | unir las 3 partes en el video final |

Manim 0.21 codifica video con PyAV, así que **no hace falta instalar `ffmpeg`
en el sistema**. Tampoco se necesita LaTeX: toda la tipografía usa `Text`
(Pango), no `Tex`.

> Manim 0.21 todavía no soporta Python 3.14. Si tu sistema trae 3.13+, usa
> `uv` (abajo) para que el entorno virtual use 3.12 sin tocar tu Python global.

---

## Instalación

```bash
git clone https://github.com/francescofq/AED-20262-Proyecto1.git
cd AED-20262-Proyecto1
```

**Opción A — con [`uv`](https://docs.astral.sh/uv/) (recomendada, descarga Python 3.12 sola):**

```bash
uv venv --python 3.12 animation/.venv
uv pip install --python animation/.venv/bin/python -r animation/requirements.txt
```

**Opción B — con un Python 3.12 ya instalado:**

```bash
python3.12 -m venv animation/.venv
animation/.venv/bin/pip install -r animation/requirements.txt
```

---

## Reproducir el video completo

Un solo comando hace todo: compila, genera los traces, renderiza las 3 partes
y las une.

```bash
./render_all.sh
```

Para previsualizar rápido en baja calidad mientras se edita:

```bash
./render_all.sh --fast
```

Salidas:

| Archivo | Contenido |
|---|---|
| `animation/media/videos/parte1/1080p60/Parte1.mp4` | Parte 1 — Qué es, TDA y propiedades |
| `animation/media/videos/parte2/1080p60/Parte2.mp4` | Parte 2 — Inserción y rebalanceo |
| `animation/media/videos/parte3/1080p60/Parte3.mp4` | Parte 3 — Casos borde, eliminación y complejidad |
| `video_final.mp4` | **el entregable**: las 3 partes concatenadas (4:26) |

### Ajustar la duración

Las dos perillas al inicio de `animation/rbviz.py` controlan el ritmo de todo
el video sin tocar ninguna escena:

```python
TEMPO_TRACE = 1.30   # velocidad de la animación del árbol
TEMPO_WAIT  = 1.00   # pausas de lectura
```

Con los valores actuales el video dura 4:26. Subir `TEMPO_TRACE` hace las
transiciones más lentas (y el video más largo); el límite del proyecto son
5 minutos.

Para saber cuánto va a durar **sin renderizar**, y de paso obtener la duración
de cada escena:

```bash
python3 animation/timing.py
```

---

## Ejecutar por separado

Solo la estructura de datos, con demo por consola:

```bash
g++ -std=c++20 -O2 main.cpp -o rbtree && ./rbtree
```

Solo regenerar los traces, sin demo:

```bash
./rbtree --quiet
```

Con CMake:

```bash
cmake -B build && cmake --build build && ./build/rbtree
```

Renderizar una sola parte (útil para trabajar en paralelo):

```bash
cd animation
.venv/bin/python -m manim render -qh parte2.py Parte2
```

---

## Estructura del repositorio

```
.
├── rbtree.hpp              Red-Black Tree (CLRS) + tracer + validate()
├── main.cpp                genera traces/*.json y la demo por consola
├── CMakeLists.txt
├── render_all.sh           pipeline completo: C++ → traces → 3 videos → final
├── guion.md                guion de narración por parte, con marcas de tiempo
├── traces/                 eventos generados por la implementación (evidencia)
└── animation/
    ├── config_video.py     ← nombres de los integrantes (EDITAR)
    ├── rbviz.py            capa de dibujo: lee los traces, no implementa nada
    ├── parte1.py           Parte 1/3
    ├── parte2.py           Parte 2/3
    ├── parte3.py           Parte 3/3
    ├── timing.py           calcula la duración de cada escena sin renderizar
    └── requirements.txt
```

---

## Contenido del video

**Parte 1 — Qué es un Red-Black Tree**
Carátula con integrantes · qué es y por qué el color · el TDA que implementa
(Conjunto/Mapa Ordenado) · las 5 propiedades · construcción real animada ·
dónde se usa (`std::map`, `TreeMap`, planificador CFS de Linux).

**Parte 2 — Inserción y rebalanceo**
Por qué el nodo nuevo entra ROJO · **Caso 1** (tío rojo: solo recoloreo) ·
**Caso 3** (tío negro, en línea: una rotación) · **Caso 2** (tío negro,
zig-zag: se reduce al Caso 3) · nueve inserciones seguidas con los tres casos
encadenándose · resumen de los tres casos.

**Parte 3 — Casos borde y complejidad**
Caso borde: árbol vacío · Caso borde: peor caso de un BST (insertar 1…8 ya
ordenado) · comparación con alturas **medidas** (100 000 claves: BST 100 000
niveles vs. RBT 31) · búsqueda · eliminación cubriendo los **4 casos** de
`fix_delete` · complejidad · conclusiones y créditos.

---

## Complejidad

| Operación | Complejidad | Detalle |
|---|---|---|
| búsqueda | O(log n) | baja un nivel por comparación |
| inserción | O(log n) | ≤ 2 rotaciones + O(log n) recoloreos |
| eliminación | O(log n) | ≤ 3 rotaciones + O(log n) recoloreos |
| espacio | O(n) | 1 bit de color por nodo |

Todo se deriva de una sola cota: **altura ≤ 2 · log₂(n + 1)**.

Alturas reales medidas por `./rbtree` insertando claves en orden creciente
(el peor caso de un BST), en `traces/p3_stats.json`:

| n | BST sin balanceo | Red-Black Tree | cota 2·log₂(n+1) |
|---:|---:|---:|---:|
| 10 | 10 | 5 | 8 |
| 100 | 100 | 11 | 14 |
| 1 000 | 1 000 | 17 | 20 |
| 10 000 | 10 000 | 24 | 28 |
| 100 000 | 100 000 | 31 | 34 |

---

## Referencias

* Cormen, Leiserson, Rivest, Stein. *Introduction to Algorithms*, 3.ª ed.,
  cap. 13 (Red-Black Trees). La implementación sigue el pseudocódigo de
  `RB-INSERT-FIXUP` y `RB-DELETE-FIXUP`.
* [Manim Community](https://docs.manim.community/) — motor de animación.
* [3Blue1Brown](https://www.3blue1brown.com/) — referencia de estilo.
