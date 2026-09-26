// ============================================================================
//  rbtree.hpp - Red-Black Tree (CLRS) instrumentado para animacion
//  CS2023 Algoritmos y Estructuras de Datos - Proyecto Final 1
//
//  La estructura es la implementacion propia del grupo. El "tracer" registra
//  cada paso atomico del algoritmo (comparacion, recoloreo, rotacion, ...) y
//  adjunta un snapshot del arbol REAL. La animacion se genera unicamente a
//  partir de estos eventos: no hay pasos dibujados a mano.
// ============================================================================
#ifndef RBTREE_HPP
#define RBTREE_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

enum Color { RED, BLACK };

// ---------------------------------------------------------------------------
// Snapshot de un nodo tal como existe en memoria en ese instante.
// depth y order NO son inventados por el animador: se calculan recorriendo
// el arbol real, asi la posicion en pantalla proviene de la estructura.
// ---------------------------------------------------------------------------
struct NodeSnap {
    int id;
    long long value;
    char color;   // 'R' | 'B'
    int parent;   // -1 = nil
    int left;     // -1 = nil
    int right;    // -1 = nil
    int depth;    // fila (0 = raiz)
    int order;    // columna (indice in-order)
};

struct Event {
    int step = 0;
    std::string op;        // "insert" | "search" | "remove"
    long long key = 0;     // clave de la operacion en curso
    std::string action;    // "op_begin","compare","attach","recolor",...
    std::string detail;    // texto explicativo para la pantalla
    std::string rb_case;   // etiqueta del caso de rebalanceo (si aplica)
    std::vector<int> focus;      // nodos a resaltar
    std::vector<int> secondary;  // nodos de contexto (tio, hermano, ...)
    std::vector<NodeSnap> nodes; // snapshot completo
    int root = -1;
    int black_height = 0;
    bool valid = true;     // se cumplen las 5 propiedades RB?
};

class Tracer {
public:
    std::vector<Event> events;
    bool enabled = true;

    void push(Event e) {
        if (!enabled) return;
        e.step = static_cast<int>(events.size());
        events.push_back(std::move(e));
    }

    // Serializacion JSON manual: sin dependencias externas.
    void write_json(const std::string& path, const std::string& title) const {
        std::ofstream out(path);
        out << "{\n";
        out << "  \"title\": \"" << title << "\",\n";
        out << "  \"events\": [\n";
        for (size_t i = 0; i < events.size(); ++i) {
            const Event& e = events[i];
            out << "    {";
            out << "\"step\": " << e.step;
            out << ", \"op\": \"" << e.op << "\"";
            out << ", \"key\": " << e.key;
            out << ", \"action\": \"" << e.action << "\"";
            out << ", \"detail\": \"" << esc(e.detail) << "\"";
            out << ", \"case\": \"" << esc(e.rb_case) << "\"";
            out << ", \"root\": " << e.root;
            out << ", \"black_height\": " << e.black_height;
            out << ", \"valid\": " << (e.valid ? "true" : "false");
            out << ", \"focus\": " << ints(e.focus);
            out << ", \"secondary\": " << ints(e.secondary);
            out << ", \"nodes\": [";
            for (size_t k = 0; k < e.nodes.size(); ++k) {
                const NodeSnap& n = e.nodes[k];
                out << "{\"id\": " << n.id
                    << ", \"value\": " << n.value
                    << ", \"color\": \"" << n.color << "\""
                    << ", \"parent\": " << n.parent
                    << ", \"left\": " << n.left
                    << ", \"right\": " << n.right
                    << ", \"depth\": " << n.depth
                    << ", \"order\": " << n.order << "}";
                if (k + 1 < e.nodes.size()) out << ", ";
            }
            out << "]}";
            if (i + 1 < events.size()) out << ",";
            out << "\n";
        }
        out << "  ]\n";
        out << "}\n";
    }

private:
    static std::string esc(const std::string& s) {
        std::string r;
        for (char c : s) {
            if (c == '"' || c == '\\') { r += '\\'; r += c; }
            else if (c == '\n') r += "\\n";
            else r += c;
        }
        return r;
    }
    static std::string ints(const std::vector<int>& v) {
        std::ostringstream os;
        os << "[";
        for (size_t i = 0; i < v.size(); ++i) {
            os << v[i];
            if (i + 1 < v.size()) os << ", ";
        }
        os << "]";
        return os.str();
    }
};

// ---------------------------------------------------------------------------
// Red-Black Tree con nodo sentinela nil (variante CLRS).
// ---------------------------------------------------------------------------
template <typename data_type>
struct RBT {
    struct Node {
        data_type data;
        Color color;
        int id;
        Node* left;
        Node* right;
        Node* parent;

        Node(data_type value, Node* nil_node, int node_id)
            : data(value), color(RED), id(node_id),
              left(nil_node), right(nil_node), parent(nil_node) {}
        Node()
            : data(data_type{}), color(BLACK), id(-1),
              left(nullptr), right(nullptr), parent(nullptr) {}
    };

    Node* root;
    Node* nil;
    int next_id = 0;
    Tracer* tracer = nullptr;   // opcional: si es nullptr la estructura corre sin instrumentar

    // operacion en curso (para etiquetar los eventos)
    std::string cur_op;
    long long cur_key = 0;

    RBT() {
        nil = new Node();
        root = nil;
    }

    ~RBT() {
        destroy(root);
        delete nil;
    }

    void destroy(Node* n) {
        if (n == nil || n == nullptr) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    // ---------------- instrumentacion ----------------

    void emit(const std::string& action, const std::string& detail,
              std::vector<int> focus = {}, std::vector<int> secondary = {},
              const std::string& rb_case = "") {
        if (!tracer || !tracer->enabled) return;
        Event e;
        e.op = cur_op;
        e.key = cur_key;
        e.action = action;
        e.detail = detail;
        e.rb_case = rb_case;
        e.focus = std::move(focus);
        e.secondary = std::move(secondary);
        e.root = (root == nil) ? -1 : root->id;
        e.black_height = black_height(root);
        e.valid = validate();
        snapshot(e.nodes);
        tracer->push(std::move(e));
    }

    void snapshot(std::vector<NodeSnap>& out) {
        int order = 0;
        collect(root, 0, order, out);
    }

    void collect(Node* n, int depth, int& order, std::vector<NodeSnap>& out) {
        if (n == nil) return;
        collect(n->left, depth + 1, order, out);
        NodeSnap s;
        s.id = n->id;
        s.value = static_cast<long long>(n->data);
        s.color = (n->color == RED) ? 'R' : 'B';
        s.parent = (n->parent == nil) ? -1 : n->parent->id;
        s.left = (n->left == nil) ? -1 : n->left->id;
        s.right = (n->right == nil) ? -1 : n->right->id;
        s.depth = depth;
        s.order = order++;
        out.push_back(s);
        collect(n->right, depth + 1, order, out);
    }

    // ---------------- verificacion de las 5 propiedades RB ----------------
    // Se usa en el video para mostrar que el invariante nunca se rompe.

    int black_height(Node* n) const {
        int h = 0;
        while (n != nil) {
            if (n->color == BLACK) ++h;
            n = n->left;
        }
        return h + 1;   // + la hoja nil, que cuenta como negra
    }

    bool validate() {
        if (root->color != BLACK) return false;   // Prop. 2: raiz negra
        if (nil->color != BLACK) return false;    // Prop. 3: hojas nil negras
        int bh = -1;
        return check(root, 0, bh);
    }

    bool check(Node* n, int blacks, int& bh) {
        if (n == nil) {
            if (bh == -1) bh = blacks + 1;
            return bh == blacks + 1;              // Prop. 5: misma altura negra
        }
        if (n->color == RED) {                    // Prop. 4: un rojo no tiene hijos rojos
            if (n->left->color == RED || n->right->color == RED) return false;
        }
        int add = (n->color == BLACK) ? 1 : 0;
        return check(n->left, blacks + add, bh) && check(n->right, blacks + add, bh);
    }

    // ---------------- rotaciones ----------------

    void rotate_left(Node* x) {
        Node* y = x->right;

        x->right = y->left;
        if (y->left != nil) y->left->parent = x;

        y->parent = x->parent;

        if (x->parent == nil) root = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;

        y->left = x;
        x->parent = y;

        emit("rotate_left", "Rotación izquierda sobre " + std::to_string((long long)x->data),
             {x->id, y->id});
    }

    void rotate_right(Node* x) {
        Node* y = x->left;

        x->left = y->right;
        if (y->right != nil) y->right->parent = x;

        y->parent = x->parent;
        if (x->parent == nil) root = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;

        y->right = x;
        x->parent = y;

        emit("rotate_right", "Rotación derecha sobre " + std::to_string((long long)x->data),
             {x->id, y->id});
    }

    // ---------------- insercion ----------------

    void fix_insert(Node* z) {
        while (z->parent->color == RED) {

            if (z->parent == z->parent->parent->left) {
                Node* y = z->parent->parent->right;   // tio

                if (y->color == RED) {
                    // Caso 1: el tio es ROJO -> solo recolorear y subir
                    emit("fixup_case",
                         "El padre y el tío son rojos: recoloreo y subo al abuelo",
                         {z->id}, {z->parent->id, y->id, z->parent->parent->id},
                         "Caso 1: tío ROJO");
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    emit("recolor", "Padre y tío a NEGRO, abuelo a ROJO",
                         {z->parent->id, y->id, z->parent->parent->id});
                    z = z->parent->parent;
                }
                else {
                    if (z == z->parent->right) {
                        // Caso 2: triangulo (zig-zag) -> lo convierto en linea
                        emit("fixup_case",
                             "Tío NEGRO y estoy en zig-zag: roto para alinearme",
                             {z->id}, {z->parent->id, z->parent->parent->id},
                             "Caso 2: tío NEGRO, zig-zag");
                        z = z->parent;
                        rotate_left(z);
                    }
                    // Caso 3: linea (zig-zig) -> recoloreo y roto el abuelo
                    emit("fixup_case",
                         "Tío NEGRO en línea: recoloreo y roto el abuelo",
                         {z->id}, {z->parent->id, z->parent->parent->id},
                         "Caso 3: tío NEGRO, zig-zig");
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    emit("recolor", "Padre a NEGRO, abuelo a ROJO",
                         {z->parent->id, z->parent->parent->id});
                    rotate_right(z->parent->parent);
                }
            }
            else {
                Node* y = z->parent->parent->left;   // tio (espejo)

                if (y->color == RED) {
                    emit("fixup_case",
                         "El padre y el tío son rojos: recoloreo y subo al abuelo",
                         {z->id}, {z->parent->id, y->id, z->parent->parent->id},
                         "Caso 1: tío ROJO");
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    emit("recolor", "Padre y tío a NEGRO, abuelo a ROJO",
                         {z->parent->id, y->id, z->parent->parent->id});
                    z = z->parent->parent;
                }
                else {
                    if (z == z->parent->left) {
                        emit("fixup_case",
                             "Tío NEGRO y estoy en zig-zag: roto para alinearme",
                             {z->id}, {z->parent->id, z->parent->parent->id},
                             "Caso 2: tío NEGRO, zig-zag");
                        z = z->parent;
                        rotate_right(z);
                    }
                    emit("fixup_case",
                         "Tío NEGRO en línea: recoloreo y roto el abuelo",
                         {z->id}, {z->parent->id, z->parent->parent->id},
                         "Caso 3: tío NEGRO, zig-zig");
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    emit("recolor", "Padre a NEGRO, abuelo a ROJO",
                         {z->parent->id, z->parent->parent->id});
                    rotate_left(z->parent->parent);
                }
            }
        }

        const Color raiz_antes = root->color;
        root->color = BLACK;
        if (raiz_antes != BLACK)
            emit("root_black", "La raíz siempre vuelve a NEGRO (propiedad 2)", {root->id});
    }

    void insert(data_type value) {
        cur_op = "insert";
        cur_key = static_cast<long long>(value);

        Node* new_node = new Node(value, nil, next_id++);

        Node* parent = nil;
        Node* current = root;

        emit("op_begin", "Insertar " + std::to_string(cur_key));

        // Descenso BST normal
        while (current != nil) {
            parent = current;
            emit("compare",
                 std::to_string(cur_key) + (new_node->data < current->data ? " < " : " >= ")
                   + std::to_string((long long)current->data) + ": voy "
                   + (new_node->data < current->data ? "a la izquierda" : "a la derecha"),
                 {current->id});
            if (new_node->data < current->data) current = current->left;
            else current = current->right;
        }

        new_node->parent = parent;

        if (parent == nil) root = new_node;
        else if (new_node->data < parent->data) parent->left = new_node;
        else parent->right = new_node;

        emit("attach", "Inserto " + std::to_string(cur_key) + " como hoja ROJA",
             {new_node->id});

        fix_insert(new_node);

        emit("op_end", "Listo: " + std::to_string(cur_key)
             + " insertado, altura negra = " + std::to_string(black_height(root)));
    }

    // ---------------- busqueda ----------------

    Node* search(Node* node, data_type key) {
        if (node == nil || key == node->data) {
            return node;
        }
        if (key < node->data) {
            return search(node->left, key);
        }
        return search(node->right, key);
    }

    Node* search(data_type key) {
        return search(root, key);
    }

    // Version instrumentada: emite un evento por cada comparacion del camino.
    Node* search_traced(data_type key) {
        cur_op = "search";
        cur_key = static_cast<long long>(key);
        emit("op_begin", "Buscar " + std::to_string(cur_key));

        Node* n = root;
        int visited = 0;
        while (n != nil && !(key == n->data)) {
            ++visited;
            emit("compare",
                 std::to_string(cur_key) + (key < n->data ? " < " : " > ")
                   + std::to_string((long long)n->data) + ": descarto medio subárbol",
                 {n->id});
            n = (key < n->data) ? n->left : n->right;
        }
        if (n == nil) {
            emit("not_found", std::to_string(cur_key) + " no está en el árbol ("
                 + std::to_string(visited) + " comparaciones)");
        } else {
            emit("found", "Encontrado " + std::to_string(cur_key) + " en "
                 + std::to_string(visited + 1) + " comparaciones", {n->id});
        }
        return n;
    }

    Node* minimum(Node* node) {
        while (node->left != nil) {
            node = node->left;
        }
        return node;
    }

    void transplant(Node* u, Node* v) {
        if (u->parent == nil) {
            root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        v->parent = u->parent;
    }

    // ---------------- eliminacion ----------------

    void fix_delete(Node* x) {
        while (x != root && x->color == BLACK) {
            if (x == x->parent->left) {
                Node* w = x->parent->right;   // hermano
                if (w->color == RED) {
                    emit("fixup_case", "Hermano ROJO: roto para volverlo NEGRO",
                         {x->id}, {w->id}, "Del. caso 1: hermano ROJO");
                    w->color = BLACK;
                    x->parent->color = RED;
                    rotate_left(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    emit("fixup_case", "Hermano NEGRO con hijos negros: subo el doble-negro",
                         {x->id}, {w->id}, "Del. caso 2: sobrinos negros");
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->right->color == BLACK) {
                        emit("fixup_case", "Sobrino lejano NEGRO: roto el hermano",
                             {x->id}, {w->id}, "Del. caso 3");
                        w->left->color = BLACK;
                        w->color = RED;
                        rotate_right(w);
                        w = x->parent->right;
                    }
                    emit("fixup_case", "Sobrino lejano ROJO: rotación final y termino",
                         {x->id}, {w->id}, "Del. caso 4");
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->right->color = BLACK;
                    rotate_left(x->parent);
                    x = root;
                }
            } else {
                Node* w = x->parent->left;    // espejo
                if (w->color == RED) {
                    emit("fixup_case", "Hermano ROJO: roto para volverlo NEGRO",
                         {x->id}, {w->id}, "Del. caso 1: hermano ROJO");
                    w->color = BLACK;
                    x->parent->color = RED;
                    rotate_right(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    emit("fixup_case", "Hermano NEGRO con hijos negros: subo el doble-negro",
                         {x->id}, {w->id}, "Del. caso 2: sobrinos negros");
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->left->color == BLACK) {
                        emit("fixup_case", "Sobrino lejano NEGRO: roto el hermano",
                             {x->id}, {w->id}, "Del. caso 3");
                        w->right->color = BLACK;
                        w->color = RED;
                        rotate_left(w);
                        w = x->parent->left;
                    }
                    emit("fixup_case", "Sobrino lejano ROJO: rotación final y termino",
                         {x->id}, {w->id}, "Del. caso 4");
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->left->color = BLACK;
                    rotate_right(x->parent);
                    x = root;
                }
            }
        }
        x->color = BLACK;
    }

    void remove(data_type data) {
        cur_op = "remove";
        cur_key = static_cast<long long>(data);
        emit("op_begin", "Eliminar " + std::to_string(cur_key));

        Node* z = search(root, data);
        if (z == nil) {
            std::cout << "Value not found in the tree." << std::endl;
            emit("not_found", std::to_string(cur_key) + " no está en el árbol");
            return;
        }
        emit("target", "Nodo a eliminar: " + std::to_string(cur_key), {z->id});

        Node* y = z;
        Node* x;
        Color y_original_color = y->color;

        if (z->left == nil) {
            x = z->right;
            transplant(z, z->right);
            emit("transplant", "Sin hijo izquierdo: lo reemplazo por su hijo derecho");
        } else if (z->right == nil) {
            x = z->left;
            transplant(z, z->left);
            emit("transplant", "Sin hijo derecho: lo reemplazo por su hijo izquierdo");
        } else {
            y = minimum(z->right);
            emit("successor", "Dos hijos: uso su sucesor in-order ("
                 + std::to_string((long long)y->data) + ")", {y->id}, {z->id});
            y_original_color = y->color;
            x = y->right;

            if (y->parent == z) {
                x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }

            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
            emit("transplant", "El sucesor toma el lugar y el color del nodo eliminado",
                 {y->id});
        }

        delete z;   // liberacion explicita de memoria

        if (y_original_color == BLACK) {
            emit("double_black",
                 "Quité un nodo NEGRO: se rompió la altura negra, hay que rebalancear");
            fix_delete(x);
        }

        emit("op_end", "Listo: " + std::to_string(cur_key)
             + " eliminado, altura negra = " + std::to_string(black_height(root)));
    }

    bool contains(data_type value) {
        return search(value) != nil;
    }

    // ---------------- impresion en consola ----------------

    void print_helper(Node* root, std::string indent, bool last) {
        if (root != nil) {
            std::cout << indent;
            if (last) {
                std::cout << "R----";
                indent += "     ";
            } else {
                std::cout << "L----";
                indent += "|    ";
            }

            std::string sColor = root->color == RED ? "RED" : "BLACK";
            std::cout << root->data << "(" << sColor << ")" << std::endl;
            print_helper(root->left, indent, false);
            print_helper(root->right, indent, true);
        }
    }

    void print() {
        if (root == nil) {
            std::cout << "Tree is empty." << std::endl;
        } else {
            print_helper(root, "", true);
        }
    }

    int height(Node* n) {
        if (n == nil) return 0;
        return 1 + std::max(height(n->left), height(n->right));
    }
    int height() { return height(root); }

    int size(Node* n) {
        if (n == nil) return 0;
        return 1 + size(n->left) + size(n->right);
    }
    int size() { return size(root); }
};

#endif // RBTREE_HPP
