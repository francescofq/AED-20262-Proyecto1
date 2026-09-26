# Guion del video — Red-Black Tree

**Duración total: 4:26** · Tres partes, una por integrante.

| Parte | Integrante | Duración | Desde – hasta |
|---|---|---|---|
| 1/3 — Qué es un Red-Black Tree | Francesco Ferrante Quino | 0:50.0 | 0:00 – 0:50.0 |
| 2/3 — Inserción y rebalanceo | Jose Ruiz Lam | 1:52.5 | 0:50.0 – 2:42.5 |
| 3/3 — Casos borde y complejidad | Osmar Vilchez Aguirre | 1:43.6 | 2:42.5 – 4:26.1 |

> Las partes no duran lo mismo: la 2 es la más larga porque contiene los tres
> casos de rebalanceo, que son el núcleo del tema. Si prefieren repartir el
> tiempo parejo, lo más fácil es mover la escena de las ocho inserciones
> seguidas de la Parte 2 a la Parte 1.

**Cómo usar este guion.** Cada bloque trae la marca de tiempo, qué se ve en
pantalla y el texto a narrar. Las duraciones salen de `animation/timing.py`,
que las calcula leyendo el código de las escenas — si cambian una escena,
vuelvan a correrlo y las marcas se actualizan.

El texto está medido para un ritmo tranquilo (~2.3 palabras por segundo). Se
indica el número de palabras de cada bloque para que puedan ajustar. **No lean
lo que ya dice la pantalla**: los subtítulos van apareciendo solos, la voz
tiene que aportar el porqué.

---

## PARTE 1 — Francesco Ferrante Quino (0:00 – 0:50)

### 0:00 · Carátula (6 s, ~14 palabras)

*En pantalla:* título, subtítulo, los tres nombres, el curso.

> Red-Black Tree: un árbol binario de búsqueda que se mantiene balanceado
> por sí solo.

### 0:06 · Qué es (5 s, ~12 palabras)

*En pantalla:* cuatro viñetas sobre el orden BST y el bit de color.

> Por dentro es un BST común. Lo distinto es que cada nodo guarda un color.

### 0:10 · El TDA (5 s, ~14 palabras)

*En pantalla:* tabla de operaciones del Conjunto Ordenado con sus costos.

> El tipo de dato que implementa es un conjunto ordenado. Insertar, borrar,
> buscar: todo logarítmico.

### 0:16 · Las 5 propiedades (8 s, ~19 palabras)

*En pantalla:* las cinco reglas y, al final, la cota de altura.

> Estas cinco reglas son todo el truco. De la cuarta y la quinta sale que la
> altura nunca pasa de dos veces el logaritmo.

### 0:23 · Construcción real (19 s, ~44 palabras)

*En pantalla:* se insertan 41, 38, 31 y 12. El panel de arriba a la derecha
muestra la altura negra y si las propiedades se cumplen.

> Miren lo que pasa al insertar. Cada nodo entra rojo, y cuando eso rompe una
> regla el árbol rota o recolorea hasta arreglarla. Arriba a la derecha pueden
> ver el indicador ponerse en ámbar mientras se repara, y volver a verde
> cuando termina. Nadie le dice cómo hacerlo: está en el algoritmo.

### 0:42 · Dónde se usa (5 s, ~13 palabras)

*En pantalla:* std::map, TreeMap, planificador de Linux, índices.

> Y no es teoría: está dentro del map de C++ y del planificador de Linux.

### 0:47 · Cierre (3 s, ~7 palabras)

> Ahora veamos cómo mantiene ese balance.

---

## PARTE 2 — Jose Ruiz Lam (0:50.0 – 2:42.5)

### 0:50 · Portada (3 s, ~6 palabras)

> Parte dos: inserción y rebalanceo.

### 0:53 · Por qué entra rojo (5 s, ~13 palabras)

*En pantalla:* comparación entre insertar un nodo rojo y uno negro.

> ¿Por qué rojo? Porque un rojo mal puesto se arregla cerca. Un negro, no.

### 0:58 · Caso 1 — tío rojo (24 s, ~55 palabras)

*En pantalla:* se insertan 10, 5, 15 y 1. Al insertar el 1 se dispara el
Caso 1 y aparece la etiqueta amarilla arriba a la izquierda.

> El nodo nuevo entró rojo y su padre también es rojo: eso rompe la regla
> cuatro. Aquí el tío, el hermano del padre, también es rojo. Cuando pasa eso
> no hace falta rotar nada: se pintan de negro el padre y el tío, el abuelo se
> pinta de rojo, y el problema sube un nivel. Si el abuelo es la raíz, se
> vuelve a pintar de negro y listo.

### 1:21 · Caso 3 — zig-zig (21 s, ~48 palabras)

*En pantalla:* se insertan 10, 20 y 30, con una rotación a la izquierda.

> Ahora el tío es negro. Fíjense en la forma: el nodo, su padre y su abuelo
> están alineados, los tres hacia el mismo lado. A eso se le llama zig-zig. Se
> resuelve con una sola rotación sobre el abuelo, más un intercambio de
> colores. Y ahí termina: no hay que seguir subiendo.

### 1:42 · Caso 2 — zig-zag (24 s, ~55 palabras)

*En pantalla:* se insertan 10, 5 y 7. Primero el Caso 2, que encadena con el 3.

> Este es el caso incómodo. El tío otra vez es negro, pero ahora el nodo y su
> padre apuntan a lados opuestos: forman un codo. Con una rotación no alcanza.
> Lo que se hace es rotar el padre para alinearlos, y con eso el problema se
> convierte exactamente en el caso anterior. Por eso el Caso 2 nunca termina
> solo: siempre desemboca en el Caso 3.

### 2:06 · Ocho inserciones seguidas (27 s, ~62 palabras)

*En pantalla:* se insertan 7, 3, 18, 10, 22, 8, 11 y 26. Las etiquetas de
caso van apareciendo según se disparan.

> Ahora todo junto, sobre un árbol que va creciendo. Vayan mirando la etiqueta
> amarilla: cada vez que se rompe una regla, el algoritmo decide cuál de los
> tres casos aplica y lo resuelve ahí mismo. Ninguna de estas decisiones está
> guionada: la animación lee los pasos que produjo nuestra implementación en
> C++ al ejecutarse.

### 2:33 · Resumen (9 s, ~21 palabras)

*En pantalla:* tabla con los tres casos.

> Resumiendo: el uno recolorea y sube, el dos alinea, el tres rota y cierra.
> Nunca más de dos rotaciones.

---

## PARTE 3 — Osmar Vilchez Aguirre (2:42.5 – 4:26.1)

### 2:42 · Portada (3 s, ~6 palabras)

> Parte tres: casos borde y complejidad.

### 2:44 · Caso borde: árbol vacío (8 s, ~18 palabras)

*En pantalla:* se inserta el primer nodo en un árbol vacío.

> El primer caso borde es el árbol vacío. El nodo entra rojo como todos, pero
> la regla dos lo obliga a ser negro.

### 2:52 · Caso borde: peor caso (27 s, ~62 palabras)

*En pantalla:* se insertan 1 a 6 en orden creciente.

> Este es el caso que mata a un BST común: insertar datos que ya vienen
> ordenados. Un árbol sin balanceo se convertiría en una lista enlazada, y
> buscar pasaría a costar lineal. Miren lo que hace este: cada vez que la
> cadena empieza a formarse, rota y la parte por la mitad. Termina con altura
> cuatro en vez de seis.

### 3:19 · Comparación medida (7 s, ~16 palabras)

*En pantalla:* tabla de alturas reales para n de 10 a 100 000.

> Y esto no son estimaciones: son alturas medidas ejecutando las dos
> estructuras.

### 3:25 · Búsqueda (11 s, ~25 palabras)

*En pantalla:* se busca el 19, que existe, y el 99, que no.

> Buscar es igual que en cualquier BST: los colores no participan. Cada
> comparación descarta la mitad del árbol.

### 3:36 · Eliminación (30 s, ~68 palabras)

*En pantalla:* se eliminan el 40 y el 20. Se disparan los cuatro casos de
`fix_delete`.

> Eliminar es bastante más difícil. Si el nodo que sale era negro, todos los
> caminos que pasaban por ahí quedan con un negro de menos y se rompe la regla
> cinco. A ese desbalance se le llama doble negro, y sube por el árbol hasta
> encontrar dónde repararse. Hay cuatro casos según el color del hermano y de
> los sobrinos. En estas dos eliminaciones se disparan los cuatro.

### 4:06 · Complejidad (6 s, ~15 palabras)

*En pantalla:* tabla de complejidades y la cota de altura.

> Todo logarítmico, y todo sale de la misma cota de altura.

### 4:12 · Conclusiones (6 s, ~15 palabras)

> Con un solo bit por nodo se garantiza altura logarítmica, y mantenerla
> cuesta muy poco.

### 4:18 · Créditos (7 s, ~16 palabras)

*En pantalla:* título, los tres nombres, el curso.

> Gracias. La animación se generó desde nuestra propia implementación en C++.

---

## Notas de grabación

- **Total ≈ 610 palabras** para 4:26, que son unas 2,3 palabras por segundo.
  Si al grabar les queda corto, alarguen las pausas; si les queda largo,
  recorten las frases explicativas, no las técnicas.
- Los bloques de tabla (TDA, comparación, complejidad, resumen) son los que
  más aguantan recorte: la pantalla ya dice lo esencial.
- Los bloques de animación (casos 1, 2 y 3, las ocho inserciones, la
  eliminación) son los que **no** conviene recortar: ahí la voz es lo que
  explica por qué pasa lo que se ve.
- Para regenerar las marcas de tiempo después de cualquier cambio:
  `python3 animation/timing.py`
