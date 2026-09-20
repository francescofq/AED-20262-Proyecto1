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

    Node* search(data_type value) {
        Node* current = root;
        while (current != nil and current->data != value) {
            if (value < current->data) current = current->left;
            else current = current->right;
        }
        return current;
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
