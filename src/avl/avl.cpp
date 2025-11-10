#include "../app.h"
#include "../render.h"
#include "../scene.h"
#include <algorithm>
#include <vector>

#include <iostream>
using namespace std;

class NodeAVL {
public:
  int data;
  NodeAVL *left;
  NodeAVL *right;
  int height;
  NodeAVL(int value) : data(value), left(nullptr), right(nullptr), height(1) {}
};

int height(NodeAVL *node) {
  if (node == nullptr) {
    return 0;
  }
  return node->height;
}

int balance(NodeAVL *node) {
  if (node == nullptr) {
    return 0;
  }
  return height(node->left) - height(node->right);
}

NodeAVL *rotateRight(NodeAVL *unbalanced) {
  NodeAVL *leftNodeAVL = unbalanced->left;
  NodeAVL *temp = leftNodeAVL->right;

  unbalanced->left = temp;
  leftNodeAVL->right = unbalanced;

  unbalanced->height =
      1 + max(height(unbalanced->left), height(unbalanced->right));
  leftNodeAVL->height =
      1 + max(height(leftNodeAVL->left), height(leftNodeAVL->right));

  return leftNodeAVL;
}

NodeAVL *rotateLeft(NodeAVL *unbalanced) {
  NodeAVL *rightNodeAVL = unbalanced->right;
  NodeAVL *temp = rightNodeAVL->left;

  unbalanced->right = temp;
  rightNodeAVL->left = unbalanced;

  unbalanced->height =
      1 + max(height(unbalanced->left), height(unbalanced->right));
  rightNodeAVL->height =
      1 + max(height(rightNodeAVL->left), height(rightNodeAVL->right));

  return rightNodeAVL;
}

NodeAVL *balanceNodeAVL(NodeAVL *node, int value) {
  int bal = balance(node);

  if (bal > 1 && node->left->data > value) {
    return rotateRight(node);
  }

  else if (bal < -1 && node->right->data < value) {
    return rotateLeft(node);
  }

  else if (bal > 1 && node->left->data < value) {
    node->left = rotateLeft(node->left);
    return rotateRight(node);
  }

  else if (bal < -1 && node->right->data > value) {
    node->right = rotateRight(node->right);
    return rotateLeft(node);
  }
  return node;
  ;
}

NodeAVL *insertAVL(NodeAVL *node, int value) {
  if (node == nullptr) {
    return new NodeAVL(value);
  }

  if (value > node->data) {
    node->right = insertAVL(node->right, value);
  } else if (value < node->data) {
    node->left = insertAVL(node->left, value);
  }

  node->height = 1 + max(height(node->left), height(node->right));

  return balanceNodeAVL(node, value);
}

NodeAVL *minValueNodeAVL(NodeAVL *node) {
  NodeAVL *current = node;
  while (current->left != nullptr) {
    current = current->left;
  }
  return current;
}

NodeAVL *deleteNodeAVL(NodeAVL *node, int value) {
  if (node == nullptr) {
    return node;
  }

  if (value < node->data) {
    node->left = deleteNodeAVL(node->left, value);
  } else if (value > node->data) {
    node->right = deleteNodeAVL(node->right, value);
  } else {
    // NodeAVL with only one child or no child
    if (node->left == nullptr || node->right == nullptr) {
      NodeAVL *temp = node->left ? node->left : node->right;
      if (temp == nullptr) {
        temp = node;
        node = nullptr;
      } else {
        *node = *temp;
      }
      delete temp;
    } else {
      // NodeAVL with two children
      NodeAVL *temp = minValueNodeAVL(node->right);
      node->data = temp->data;
      node->right = deleteNodeAVL(node->right, temp->data);
    }
  }

  if (node == nullptr) {
    return node;
  }

  node->height = 1 + max(height(node->left), height(node->right));
  return balanceNodeAVL(node, value);
}

struct AVLImpl {
  using Node = NodeAVL;
  Node *r = nullptr;

  const char *title() const { return "AVL Tree"; }

  Node *root() const { return r; }
  Node *left(Node *n) const { return n ? n->left : nullptr; }
  Node *right(Node *n) const { return n ? n->right : nullptr; }

  void draw_label(int x, int y, Node *n) const {
    draw_node_label(x, y, n->data);
  }

  void insert(int k) { r = insertAVL(r, k); }
  void erase(int k) { r = deleteNodeAVL(r, k); }

  void clear() {
    vector<Node *> st;
    if (r)
      st.push_back(r);
    while (!st.empty()) {
      Node *n = st.back();
      st.pop_back();
      if (n->left)
        st.push_back(n->left);
      if (n->right)
        st.push_back(n->right);
      delete n;
    }
    r = nullptr;
  }

  vector<int> sample() const { return {30, 20, 40, 10, 25, 35, 50, 5, 15, 27}; }
};

// single global instance + factory using the common generic scene
static TreeScene<AVLImpl> g_avl_scene;
Scene *make_avl_scene() { return &g_avl_scene; };
