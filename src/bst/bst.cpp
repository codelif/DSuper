#include "../app.h"
#include "../render.h"
#include "../scene.h"
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include <iostream>
using namespace std;

class NodeBST {
public:
  int data;
  NodeBST *left;
  NodeBST *right;
  NodeBST(int value) : data(value), left(nullptr), right(nullptr) {}
};

NodeBST *insertAVL(NodeBST *node, int value) {
  if (node == nullptr) {
    return new NodeBST(value);
  }
  if (value > node->data) {
    node->right = insertAVL(node->right, value);
  } else if (value < node->data) {
    node->left = insertAVL(node->left, value);
  }
  return node;
}

NodeBST *minValueNodeBST(NodeBST *node) {
  NodeBST *current = node;
  while (current->left != nullptr) {
    current = current->left;
  }
  return current;
}

NodeBST *deleteNodeBST(NodeBST *root, int value) {
  if (root == nullptr) {
    return root;
  }

  if (value < root->data) {
    root->left = deleteNodeBST(root->left, value);
  } else if (value > root->data) {
    root->right = deleteNodeBST(root->right, value);
  } else {
    // NodeBST with only one child or no child
    if (root->left == nullptr) {
      NodeBST *temp = root->right;
      delete root;
      return temp;
    } else if (root->right == nullptr) {
      NodeBST *temp = root->left;
      delete root;
      return temp;
    }

    // NodeBST with two children
    NodeBST *temp = minValueNodeBST(root->right);
    root->data = temp->data;
    root->right = deleteNodeBST(root->right, temp->data);
  }

  return root;
}

struct BSTImpl {
  using Node = NodeBST;
  Node *r = nullptr;

  const char *title() const { return "BST Tree"; }

  Node *root() const { return r; }
  Node *left(Node *n) const { return n ? n->left : nullptr; }
  Node *right(Node *n) const { return n ? n->right : nullptr; }

  void draw_label(int x, int y, Node *n) const {
    draw_node_label(x, y, n->data);
  }

  void insert(int k) { r = insertAVL(r, k); }
  void erase(int k) { r = deleteNodeBST(r, k); }

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
static TreeScene<BSTImpl> g_bst_scene;
Scene *make_bst_scene() { return &g_bst_scene; }
