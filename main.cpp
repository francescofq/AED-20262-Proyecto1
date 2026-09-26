// ============================================================================
//  main.cpp - Generador de traces para la animacion del Red-Black Tree
//  CS2023 Algoritmos y Estructuras de Datos - Proyecto Final 1
//
//  Cada escena del video tiene su propio trace. Este programa ejecuta la
//  implementacion REAL del arbol y volca los pasos a traces/*.json.
//  La animacion (Manim) solo lee esos archivos: no reimplementa nada.
//
//  Uso:  ./rbtree            -> genera traces/ y una demo por consola
//        ./rbtree --quiet    -> solo genera traces/
// ============================================================================
#include "rbtree.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

// ---------------------------------------------------------------------------
// BST sin balanceo. NO es la estructura asignada: se usa solo para medir
// cuanto degenera un BST comun y contrastarlo con el RBT en la Parte 3.
// ---------------------------------------------------------------------------
struct PlainBST {
    struct N { int v; N* l = nullptr; N* r = nullptr; N(int x) : v(x) {} };
    N* root = nullptr;
    int max_depth = 0;

    // Con claves ya ordenadas este arbol degenera en una cadena de n nodos.
    // Por eso ni la altura ni la destruccion pueden ser recursivas: con
    // n = 100000 serian 100000 niveles de pila y se desborda.
    void insert(int x) {
        N** cur = &root;
        int depth = 1;
        while (*cur) {
            cur = (x < (*cur)->v) ? &(*cur)->l : &(*cur)->r;
            ++depth;
        }
        *cur = new N(x);
        max_depth = std::max(max_depth, depth);
    }

    int height() const { return max_depth; }

    ~PlainBST() {
        std::vector<N*> stack;
        if (root) stack.push_back(root);
        while (!stack.empty()) {
            N* n = stack.back();
            stack.pop_back();
            if (n->l) stack.push_back(n->l);
            if (n->r) stack.push_back(n->r);
            delete n;
        }
    }
};

// Construye un arbol, guarda el trace y devuelve el arbol listo para seguir usandolo.
static void build_and_dump(const std::vector<int>& values,
                           const std::string& path,
                           const std::string& title) {
    RBT<int> t;
    Tracer tr;
    t.tracer = &tr;
    for (int v : values) t.insert(v);
    tr.write_json(path, title);
    std::cout << "  " << path << "  (" << tr.events.size() << " eventos, "
              << values.size() << " claves, altura " << t.height() << ")\n";
}

int main(int argc, char** argv) {
    bool quiet = (argc > 1 && std::string(argv[1]) == "--quiet");

    std::filesystem::create_directories("traces");
    std::cout << "Generando traces...\n";

    // ---------------- PARTE 1: introduccion y TDA ----------------
    // Construccion completa: se ve como el arbol se mantiene balanceado solo.
    build_and_dump({41, 38, 31, 12, 19, 8},
                   "traces/p1_construccion.json",
                   "Parte 1 - Construcción de un Red-Black Tree");

    // ---------------- PARTE 2: los tres casos de fix_insert ----------------
    // Cada secuencia esta elegida para disparar un caso concreto en su ULTIMA
    // insercion. El propio trace registra la etiqueta del caso, no la narracion.
    build_and_dump({10, 20, 30},
                   "traces/p2_caso3_zigzig.json",
                   "Parte 2 - Caso 3: tío NEGRO en línea (zig-zig)");

    build_and_dump({10, 5, 7},
                   "traces/p2_caso2_zigzag.json",
                   "Parte 2 - Caso 2: tío NEGRO en zig-zag");

    build_and_dump({10, 5, 15, 1},
                   "traces/p2_caso1_tio_rojo.json",
                   "Parte 2 - Caso 1: tío ROJO (solo recoloreo)");

    // Secuencia larga: se ven los tres casos encadenandose en una corrida real.
    build_and_dump({7, 3, 18, 10, 22, 8, 11, 26, 2, 6, 13},
                   "traces/p2_secuencia.json",
                   "Parte 2 - Once inserciones seguidas");

    // ---------------- PARTE 3: casos borde, busqueda, eliminacion ----------------
    // Caso borde A: arbol vacio -> el primer nodo debe quedar NEGRO.
    build_and_dump({7},
                   "traces/p3_borde_vacio.json",
                   "Parte 3 - Caso borde: árbol vacío");

    // Caso borde B: peor caso de un BST (insercion ya ordenada).
    std::vector<int> ordenado;
    for (int i = 1; i <= 10; ++i) ordenado.push_back(i);
    build_and_dump(ordenado,
                   "traces/p3_borde_ordenado.json",
                   "Parte 3 - Caso borde: inserción en orden creciente 1..10");

    // Busqueda instrumentada sobre un arbol ya construido.
    {
        RBT<int> t;
        Tracer tr;
        t.tracer = &tr;
        tr.enabled = false;                       // la construccion no se anima aqui
        for (int v : {41, 38, 31, 12, 19, 8, 45, 50, 2}) t.insert(v);
        tr.enabled = true;
        t.search_traced(19);                      // exito
        t.search_traced(99);                      // fracaso
        tr.write_json("traces/p3_busqueda.json", "Parte 3 - Búsqueda");
        std::cout << "  traces/p3_busqueda.json  (" << tr.events.size() << " eventos)\n";
    }

    // Eliminacion de un nodo NEGRO: aparece el problema del "doble negro".
    {
        RBT<int> t;
        Tracer tr;
        t.tracer = &tr;
        tr.enabled = false;
        // Secuencia elegida porque dispara los CUATRO casos de fix_delete
        // (hermano rojo, sobrinos negros, sobrino lejano negro y sobrino
        // lejano rojo). Se verifico ejecutando la propia implementacion.
        for (int v : {20, 35, 40, 10, 25, 30, 15, 5}) t.insert(v);
        tr.enabled = true;
        t.remove(40);
        t.remove(20);
        tr.write_json("traces/p3_eliminacion.json", "Parte 3 - Eliminación y doble negro");
        std::cout << "  traces/p3_eliminacion.json  (" << tr.events.size() << " eventos)\n";
    }

    // ---------------- Estadisticas reales: RBT vs BST sin balanceo ----------------
    {
        std::ofstream out("traces/p3_stats.json");
        out << "{\n  \"comparacion\": [\n";
        std::vector<int> ns = {10, 100, 1000, 10000, 100000};
        for (size_t i = 0; i < ns.size(); ++i) {
            int n = ns[i];
            RBT<int> rbt;
            PlainBST bst;
            for (int k = 1; k <= n; ++k) { rbt.insert(k); bst.insert(k); }
            int cota = 0;                       // cota teorica 2*log2(n+1)
            { long long p = 1; int lg = 0; while (p < (long long)n + 1) { p *= 2; ++lg; } cota = 2 * lg; }
            out << "    {\"n\": " << n
                << ", \"altura_rbt\": " << rbt.height()
                << ", \"altura_bst\": " << bst.height()
                << ", \"altura_negra\": " << rbt.black_height(rbt.root)
                << ", \"cota_2log2\": " << cota
                << ", \"propiedades_ok\": " << (rbt.validate() ? "true" : "false") << "}";
            if (i + 1 < ns.size()) out << ",";
            out << "\n";
        }
        out << "  ]\n}\n";
        std::cout << "  traces/p3_stats.json  (RBT vs BST sin balanceo)\n";
    }

    if (quiet) return 0;

    // ---------------- Demo por consola ----------------
    std::cout << "\n=== Demo por consola ===\n";
    RBT<int> demo;
    for (int v : {10, 20, 30, 15, 25, 5}) {
        std::cout << "\nInsertando " << v << "...\n";
        demo.insert(v);
        demo.print();
    }
    std::cout << "\nPropiedades RB se cumplen: " << (demo.validate() ? "SI" : "NO") << "\n";
    std::cout << "Altura = " << demo.height()
              << ", altura negra = " << demo.black_height(demo.root)
              << ", nodos = " << demo.size() << "\n";

    std::cout << "\nEliminando 20...\n";
    demo.remove(20);
    demo.print();
    std::cout << "Propiedades RB se cumplen: " << (demo.validate() ? "SI" : "NO") << "\n";
    return 0;
}
