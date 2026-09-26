#ifndef RBT_HPP
#define RBT_HPP

// Implementacion del integrante: José Ruiz

#include <cstddef>

namespace utec {
    template <typename T>
    struct RBT {
        struct Node {
            T data;
            Node* right;
            Node* left;
            Node* padre;
            char color; // r | b (referencia a red or black)
            Node (const T& x, const char& c, Node* nil) : data(x), right(nil), left(nil), padre(nil), color(c) {}
            Node () : right(nullptr), left(nullptr), padre(nullptr), color('b') {}
            ~Node(){
                right = nullptr;
                left = nullptr;
                padre = nullptr;
            }
        };

        Node* root;
        Node* nil;
        size_t sz = 0;

        RBT() {
            nil = new Node ();
            root = nil;
            sz = 0;
        }
        RBT(const T& x) : RBT() {
            insert(x);
        }

        ~RBT() {
            clear();
            delete nil;
        }

        bool empty () {
            return sz == 0;
        }

        size_t size () {
            return sz;
        }

        T* search (const T& x) {
            Node* answer = search(x, root);
            if (answer == nil) return nullptr;
            return &(answer->data);
        }

        void clear () {
            clear (root);
            root = nil;
            sz = 0;
        }

        T* minimum () {
            Node* answer = minimum (root);
            if (answer == nil) return nullptr;
            else return &(answer->data);
        }

        T* maximum () {
            Node* answer = maximum (root);
            if (answer == nil) return nullptr;
            else return &(answer->data);
        }

        void insert(const T& k) {
            Node* padre = nil;
            Node* actual = root;

            while (actual != nil) {
                if (k == actual->data) return;
                padre = actual;
                if (k < actual->data) {
                    actual = actual->left;
                } else {
                    actual = actual->right;
                }
            }
            Node* nuevo = new Node(k, 'r', nil);
            nuevo->padre = padre;

            if (padre == nil) {
                root = nuevo;
            } else if (k < padre->data) {
                padre->left = nuevo;
            } else {
                padre->right = nuevo;
            }

            ++sz;
            fix_insert(nuevo);
        }

        T* successor (const T& k) {
            Node* current = search(k, root);
            if (current == nil) return nullptr;
            if (current->right != nil) {
                return &(minimum(current->right)->data);
            }
            Node* padre = current->padre;
            while (padre != nil and current == padre->right) {
                current = padre;
                padre = padre->padre;
            }
            if (padre == nil) return nullptr;
            return &(padre->data);
        }

        T* predecessor (const T& k) {
            Node* current = search(k, root);
            if (current == nil) return nullptr;
            if (current->left != nil) {
                return &(maximum(current->left)->data);
            }
            Node* padre = current->padre;
            while (padre != nil and current == padre->left) {
                current = padre;
                padre = padre->padre;
            }
            if (padre == nil) return nullptr;
            return &(padre->data);
        }

        void erase(const T& k) {
            Node* z = search(k, root);
            if (z == nil) return;

            Node* y = z;
            Node* x = nil;
            char y_color = y->color;

            if (z->left == nil) {
                x = z->right;
                transplant(z, z->right);
            } else if (z->right == nil) {
                x = z->left;
                transplant(z, z->left);
            } else {
                y = minimum(z->right);
                y_color = y->color;
                x = y->right;

                if (y->padre == z) {
                    x->padre = y;
                } else {
                    transplant(y, y->right);
                    y->right = z->right;
                    y->right->padre = y;
                }

                transplant(z, y);
                y->left = z->left;
                y->left->padre = y;
                y->color = z->color;
            }

            delete z;
            --sz;

            if (y_color == 'b') {
                fix_erase(x);
            }
        }

        private:
        Node* search (const T& x, Node* node) {
            if (node == nil or x == node->data) return node;
            if (x < node->data) return search(x, node->left);
            return search(x, node->right);
        }

        void transplant(Node* u, Node* v) {
            if (u->padre == nil) {
                root = v;
            } else if (u == u->padre->left) {
                u->padre->left = v;
            } else {
                u->padre->right = v;
            }
            v->padre = u->padre;
        }

        void clear (Node* current) {
            if (current == nil) return;
            clear(current->left);
            clear(current->right);
            delete current;
        }

        Node* minimum (Node* current) {
            if (current != nil) {
                while (current->left != nil){
                    current = current->left;
                }
            }
            return current;
        }

        Node* maximum (Node* current) {
            if (current != nil) {
                while (current->right != nil){
                    current = current->right;
                }
            }
            return current;
        }

        void left_rotation(Node* node) {
            if (node == nil || node->right == nil) return;

            Node* node_right = node->right;
            node->right = node_right->left;

            if (node->right != nil) {
                node->right->padre = node;
            }

            node_right->padre = node->padre;

            if (node->padre == nil) {
                root = node_right;
            } else if (node == node->padre->left) {
                node->padre->left = node_right;
            } else {
                node->padre->right = node_right;
            }

            node_right->left = node;
            node->padre = node_right;
        }

        void right_rotation(Node* node) {
            if (node == nil || node->left == nil) return;

            Node* node_left = node->left;

            node->left = node_left->right;
            if (node->left != nil) {
                node->left->padre = node;
            }

            node_left->padre = node->padre;
            if (node->padre == nil) {
                root = node_left;
            } else if (node == node->padre->left) {
                node->padre->left = node_left;
            } else {
                node->padre->right = node_left;
            }

            node_left->right = node;
            node->padre = node_left;
        }

        void fix_insert (Node* current) {
            if (current->padre == nil or current->padre->color == 'b') {
                root->color = 'b';
                return;
            }

            Node* padre = current->padre;
            Node* abuelo = padre->padre;
            Node* tio = (abuelo->right == padre) ? abuelo->left : abuelo->right;

            // Case 1 y 2: tio is red
            if (tio->color == 'r') {
                // Step 1: padre and uncle to black
                padre->color = 'b';
                tio->color = 'b';
                // Step 2: abuelo to red
                abuelo->color = 'r';
                // Step 3: current = abuelo
                return fix_insert(abuelo);
            }

            // Case 3: tio is black, padre is abuelo's left child, current is padre's left child
            if (padre == abuelo->left and current == padre->left) {
                // Step 1: padre to black
                padre->color = 'b';
                // Step 2: abuelo to red
                abuelo->color = 'r';
                // Step 3: right rotate on abuelo
                right_rotation(abuelo);
            }
            // Case 4: tio is black, padre is abuelo's right child, current is padre's right child
            else if (padre == abuelo->right and current == padre->right) {
                // Step 1: padre to black
                padre->color = 'b';
                // Step 2: abuelo to red
                abuelo->color = 'r';
                // Step 3: left rotate on abuelo
                left_rotation(abuelo);
            }
            // Case 5: tio is black, padre is abuelo's left child, current is padre's right child
            else if (padre == abuelo->left and current == padre->right) {
                // Step 1: left rotate on padre
                left_rotation(padre);
                // Step 2: current to black
                current->color = 'b';
                // Step 3: abuelo to red
                abuelo->color = 'r';
                // Step 4: right rotate on abuelo
                right_rotation(abuelo);
            }
            // Case 6: tio is black, padre is abuelo's right child, current is padre's left child
            else if (padre == abuelo->right and current == padre->left) {
                // Step 1: right rotate on padre
                right_rotation(padre);
                // Step 2: current to black
                current->color = 'b';
                // Step 3: abuelo to red
                abuelo->color = 'r';
                // Step 4: left rotate on abuelo
                left_rotation(abuelo);
            }
        }

        void fix_erase(Node* x) {
            while (x != root && x->color == 'b') {
                if (x == x->padre->left) {
                    Node* w = x->padre->right;
                    // Case 1: El hermano 'w' es rojo
                    if (w->color == 'r') {
                        w->color = 'b';
                        x->padre->color = 'r';
                        left_rotation(x->padre);
                        w = x->padre->right;
                    }
                    // Case 2: Ambos hijos de 'w' son negros
                    if (w->left->color == 'b' && w->right->color == 'b') {
                        w->color = 'r';
                        x = x->padre;
                    } else {
                        // Case 3: El hijo derecho de 'w' es negro (hijo izquierdo rojo)
                        if (w->right->color == 'b') {
                            w->left->color = 'b';
                            w->color = 'r';
                            right_rotation(w);
                            w = x->padre->right;
                        }
                        // Case 4: El hijo derecho de 'w' es rojo
                        w->color = x->padre->color;
                        x->padre->color = 'b';
                        w->right->color = 'b';
                        left_rotation(x->padre);
                        x = root;
                    }
                } else { // Case simétrico (x es el hijo derecho)
                    Node* w = x->padre->left;
                    // Case 1
                    if (w->color == 'r') {
                        w->color = 'b';
                        x->padre->color = 'r';
                        right_rotation(x->padre);
                        w = x->padre->left;
                    }
                    // Case 2
                    if (w->right->color == 'b' && w->left->color == 'b') {
                        w->color = 'r';
                        x = x->padre;
                    } else {
                        // Case 3
                        if (w->left->color == 'b') {
                            w->right->color = 'b';
                            w->color = 'r';
                            left_rotation(w);
                            w = x->padre->left;
                        }
                        // Case 4
                        w->color = x->padre->color;
                        x->padre->color = 'b';
                        w->left->color = 'b';
                        right_rotation(x->padre);
                        x = root;
                    }
                }
            }
            x->color = 'b';
        }
    };
}

#endif
