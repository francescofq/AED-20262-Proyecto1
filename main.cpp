#include <bits/stdc++.h>
using namespace std;

enum Color { RED, BLACK };

template <typename data_type>
struct RBT {
    struct Node {
        data_type data;
        Color color;

        Node* left;
        Node* right;
        Node* parent;

        Node(data_type value, Node* nil_node): data(value), color(RED), left(nil_node), right(nil_node), parent(nil_node) {}
        Node(): color(BLACK), left(nullptr), right(nullptr), parent(nullptr) {}
    };

    Node* root;
    Node* nil;

    RBT() {
        nil = new Node();
        root = nil;
    }

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
    }

void fix_insert(Node* z) {
        while (z->parent->color == RED) {

            if (z->parent == z->parent->parent->left) {
                Node* y = z->parent->parent->right;

                // Caso 1: Tio es ROJO
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent; // Subimos al abuelo
                }
                else {
                    // Caso 2: Tio es NEGRO y 'z' es hijo derecho
                    if (z == z->parent->right) {
                        z = z->parent;
                        rotate_left(z);
                    }
                    // Caso 3: Tio es NEGRO y 'z' es hijo izquierdo
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rotate_right(z->parent->parent);
                }
            }
            else {
                Node* y = z->parent->parent->left;

                // Caso 1: Tio es ROJO
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent; // Subimos al abuelo
                }
                else {
                    // Caso 2: Tio es NEGRO y 'z' es hijo izquierdo
                    if (z == z->parent->left) {
                        z = z->parent;
                        rotate_right(z);
                    }
                    // Caso 3: Tio es NEGRO y 'z' es hijo derecho
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rotate_left(z->parent->parent);
                }
            }
        }

        root->color = BLACK;
    }

    void insert(data_type value) {
        Node* new_node = new Node(value, nil);

        Node* parent = nil;
        Node* current = root;

        //BST normal
        while (current != nil) {
            parent = current;
            if (new_node->data < current->data) current = current->left;
            else current = current->right;
        }

        new_node->parent = parent;

        if (parent == nil) root = new_node;
        else if (new_node->data < parent->data) parent->left = new_node;
        else parent->right = new_node;

        fix_insert(new_node);
    }

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

    // Rebalanceo tras la eliminación (traducido de fix_delete)
    void fix_delete(Node* x) {
        while (x != root && x->color == BLACK) {
            if (x == x->parent->left) {
                Node* w = x->parent->right;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rotate_left(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->right->color == BLACK) {
                        w->left->color = BLACK;
                        w->color = RED;
                        rotate_right(w);
                        w = x->parent->right;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->right->color = BLACK;
                    rotate_left(x->parent);
                    x = root;
                }
            } else {
                Node* w = x->parent->left;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rotate_right(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->left->color == BLACK) {
                        w->right->color = BLACK;
                        w->color = RED;
                        rotate_left(w);
                        w = x->parent->left;
                    }
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

    // Eliminación del nodo (traducido de delete)
    void remove(data_type data) {
        Node* z = search(root, data);
        if (z == nil) {
            cout << "Value not found in the tree." << endl;
            return;
        }

        Node* y = z;
        Node* x;
        Color y_original_color = y->color;

        if (z->left == nil) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nil) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = minimum(z->right);
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
        }

        delete z; // Liberación explícita de memoria requerida en C++

        if (y_original_color == BLACK) {
            fix_delete(x);
        }
    }

    bool contains(data_type value) {
        return search(value) != nil;
    }

    void print_helper(Node* root, string indent, bool last) {
        if (root != nil) {
            cout << indent;
            if (last) {
                cout << "R----";
                indent += "     ";
            } else {
                cout << "L----";
                indent += "|    ";
            }

            string sColor = root->color == RED ? "RED" : "BLACK";
            cout << root->data << "(" << sColor << ")" << endl;
            print_helper(root->left, indent, false);
            print_helper(root->right, indent, true);
        }
    }

    void print() {
        if (root == nil) {
            cout << "Tree is empty." << endl;
        } else {
            print_helper(root, "", true);
        }
    }
};

int main() {
    RBT<int> myTree;

    int values[] = {10, 20, 30, 15, 25, 5};

    for(int val : values) {
        cout << "\nInserting " << val << "...\n";
        myTree.insert(val);
        myTree.print();
    }
    return 0;
}
