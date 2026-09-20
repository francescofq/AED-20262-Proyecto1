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


};

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
