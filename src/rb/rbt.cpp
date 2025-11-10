#include "../app.h"
#include "../render.h"
#include "../scene.h"
#include <sstream>
#include <string>
#include <vector>

#include <iostream>
using namespace std;

class NodeRBT {
public:
  int data;
  NodeRBT *parent;
  NodeRBT *left;
  NodeRBT *right;
  char color;
  NodeRBT(int value)
      : data(value), parent(nullptr), left(nullptr), right(nullptr),
        color('R') {}
};

struct RBTImpl {
  using Node = NodeRBT;
  Node *root_ = nullptr;

  Node *root() { return root_; }
  Node *root() const { return root_; }

  const char *title() const { return "Red-Black Tree"; }

  Node *left(Node *n) const { return n ? n->left : nullptr; }
  Node *right(Node *n) const { return n ? n->right : nullptr; }

  void draw_label(int cx, int cy, Node *n) const {
    // prints [keyR] with R in red, or [keyB]
    ostringstream ss;
    if (n->color == 'R')
      ss << '[' << n->data << "\x1b[31mR\x1b[0m]";
    else
      ss << '[' << n->data << "B]";
    string s = ss.str();
    int x = cx - (int)s.size() / 2;
    printxy(x, cy, s);
  }

  void insert(int value) {
    Node *n = new Node(value);
    if (!root_) {
      root_ = n;
      root_->color = 'B';
      return;
    }
    Node *p = nullptr, *c = root_;
    while (c) {
      p = c;
      if (c->data > value)
        c = c->left;
      else
        c = c->right;
    }
    if (p->data > value) {
      p->left = n;
      n->parent = p;
    } else {
      p->right = n;
      n->parent = p;
    }
    insertfix(n);
  }

  void clear() {
    vector<Node *> st;
    if (root_)
      st.push_back(root_);
    while (!st.empty()) {
      Node *n = st.back();
      st.pop_back();
      if (n->left)
        st.push_back(n->left);
      if (n->right)
        st.push_back(n->right);
      delete n;
    }
    root_ = nullptr;
  }

  void erase(int k) {}

  vector<int> sample() const { return {8, 18, 5, 15, 17, 25, 40, 80}; }

private:
  Node *grandparent(Node *node) {
    if (node && node->parent)
      return node->parent->parent;
    return nullptr;
  }

  Node *uncle(Node *node) {
    Node *g = grandparent(node);
    if (!g)
      return nullptr;
    if (g->left == node->parent)
      return g->right;
    else
      return g->left;
  }

  void leftRotate(Node *g) {
    Node *p = g->right;
    Node *t = p->left;
    g->right = t;
    if (t)
      t->parent = g;
    p->parent = g->parent;
    if (g->parent) {
      if (g->parent->left == g)
        g->parent->left = p;
      else
        g->parent->right = p;
    } else
      root_ = p;
    p->left = g;
    g->parent = p;
  }

  void rightRotate(Node *g) {
    Node *p = g->left;
    Node *t = p->right;
    g->left = t;
    if (t)
      t->parent = g;
    p->parent = g->parent;
    if (g->parent) {
      if (g->parent->left == g)
        g->parent->left = p;
      else
        g->parent->right = p;
    } else
      root_ = p;
    p->right = g;
    g->parent = p;
  }

  void insertfix(Node *n) {
    if (root_ == n) {
      n->color = 'B';
      return;
    }
    if (n->parent->color == 'B')
      return;
    Node *u = uncle(n);
    Node *g = grandparent(n);
    Node *p = n->parent;
    if (u && u->color == 'R') {
      u->color = 'B';
      p->color = 'B';
      g->color = 'R';
      insertfix(g);
      return;
    }
    if (p == g->left && n == p->right) {
      leftRotate(p);
      n = n->left;
      p = n->parent;
    } else if (p == g->right && n == p->left) {
      rightRotate(p);
      n = n->right;
      p = n->parent;
    }
    if (n == p->left)
      rightRotate(g);
    else
      leftRotate(g);
    p->color = 'B';
    g->color = 'R';
  }
};

// single global instance + factory using the common generic scene
static TreeScene<RBTImpl> g_rbt_scene;
Scene *make_rbt_scene() { return &g_rbt_scene; }
