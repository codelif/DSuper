#include "app.h"
#include "scene.h"
#include "tui.h"
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#include <iostream>
using namespace std;
class rbtree {
public:
  class Node {
  public:
    int data;
    Node *parent;
    Node *left;
    Node *right;
    char color;
    Node(int value)
        : data(value), parent(nullptr), left(nullptr), right(nullptr),
          color('R') {}
  };

  Node *root;
  rbtree() : root(nullptr) {}

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
      root = p;
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
      root = p;
    p->right = g;
    g->parent = p;
  }

  void insertfix(Node *n) {
    if (root == n) {
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

  void insert(int value) {
    Node *n = new Node(value);
    if (!root) {
      root = n;
      root->color = 'B';
      return;
    }
    Node *p = nullptr, *c = root;
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

  void inOrder(Node *r) {
    if (!r)
      return;
    inOrder(r->left);
    cout << r->data << r->color << " ";
    inOrder(r->right);
  }
};

struct Pos {
  rbtree::Node *n;
  int x;
  int y;
};

static void set_pos(vector<Pos> &a, rbtree::Node *n, int x, int y) {
  for (auto &p : a)
    if (p.n == n) {
      p.x = x;
      p.y = y;
      return;
    }
  a.push_back({n, x, y});
}
static bool get_pos(const vector<Pos> &a, rbtree::Node *n, int &x, int &y) {
  for (auto &p : a)
    if (p.n == n) {
      x = p.x;
      y = p.y;
      return true;
    }
  return false;
}
static void compute_layout(rbtree::Node *r, int x1, int x2, int y, int ys,
                           vector<Pos> &pos) {
  if (!r)
    return;
  int mid = (x1 + x2) / 2;
  set_pos(pos, r, mid, y);
  if (r->left)
    compute_layout(r->left, x1, mid - 4, y + ys, ys, pos);
  if (r->right)
    compute_layout(r->right, mid + 4, x2, y + ys, ys, pos);
}

static inline string red(const string &s) { return "\x1b[31m" + s + "\x1b[0m"; }
static void draw_node_label_color(int cx, int cy, int key, char c) {
  ostringstream ss;
  if (c == 'R')
    ss << '[' << key << red("R") << ']';
  else
    ss << '[' << key << "B]";
  string s = ss.str();
  int x = cx - (int)s.size() / 2;
  printxy(x, cy, s);
}

class RBTSceneImpl : public Scene {
  rbtree T;
  string buf;
  vector<int> hist;
  int hist_max = 8;
  void push_hist(int k) {
    if ((int)hist.size() == hist_max)
      hist.erase(hist.begin());
    hist.push_back(k);
  }
  void clear_tree() {
    vector<rbtree::Node *> st;
    if (T.root)
      st.push_back(T.root);
    while (!st.empty()) {
      auto n = st.back();
      st.pop_back();
      if (n->left)
        st.push_back(n->left);
      if (n->right)
        st.push_back(n->right);
      delete n;
    }
    T.root = nullptr;
  }

public:
  ~RBTSceneImpl() { clear_tree(); }
  const char *title() const { return "Red-Black Tree"; }

  void on_key(int key) {
    static int last_q = 0;
    if (key == 'q') {
      if (last_q == 'q') {
        request_quit();
        return;
      }
      last_q = 'q';
    } else
      last_q = 0;
    if (key == KEY_ESC || key == 'b') {
      buf.clear();
      set_scene(make_menu_scene());
      return;
    }
    if (key == 'c') {
      clear_tree();
      buf.clear();
    } else if (key >= '0' && key <= '9') {
      if (buf.size() < 9)
        buf.push_back((char)key);
    } else if (key == 127 || key == '\b') {
      if (!buf.empty())
        buf.pop_back();
    } else if (key == '\n') {
      if (!buf.empty()) {
        int k = atoi(buf.c_str());
        buf.clear();
        T.insert(k);
        push_hist(k);
      }
    } else if (key == 'r') {
      int a[] = {8, 18, 5, 15, 17, 25, 40, 80};
      for (int v : a)
        T.insert(v);
    }
  }

  void render() {
    Winsize ws = get_term_size();
    int W = ws.width, H = ws.height;
    clear_scr();
    frame(0, 0, W - 1, H - 1);
    string bar = " Red-Black Tree (insert only) ";
    frame(2, 1, (int)bar.size() + 2, 3);
    printxy(3, 2, bar);
    int cpw = min(48, max(30, W / 3));
    frame(2, 5, cpw, 7);
    printxy(4, 6, string("Input: ") + (buf.empty() ? "_" : buf));
    printxy(4, 7, "[Enter] insert   [c] clear   [r] sample");
    printxy(4, 8, "[b/Esc] back     [q q] quit");
    frame(2, 13, cpw, 5);
    string h = "History: ";
    for (int k : hist)
      h += to_string(k) + " ";
    printxy(4, 14, h);
    int dx = cpw + 3, dw = W - dx - 3, dy = 5, dh = H - dy - 3;
    frame(dx, dy, dw, dh);
    if (T.root) {
      vector<Pos> pos;
      int x1 = dx + 3, x2 = dx + dw - 4, y1 = dy + 2, ys = 3;
      compute_layout(T.root, x1, x2, y1, ys, pos);
      vector<rbtree::Node *> st;
      st.push_back(T.root);
      while (!st.empty()) {
        auto n = st.back();
        st.pop_back();
        int x = 0, y = 0;
        get_pos(pos, n, x, y);
        if (n->left) {
          int xl = 0, yl = 0;
          get_pos(pos, n->left, xl, yl);
          draw_connector(x, y, xl, yl);
          st.push_back(n->left);
        }
        if (n->right) {
          int xr = 0, yr = 0;
          get_pos(pos, n->right, xr, yr);
          draw_connector(x, y, xr, yr);
          st.push_back(n->right);
        }
      }
      st.clear();
      st.push_back(T.root);
      while (!st.empty()) {
        auto n = st.back();
        st.pop_back();
        int x = 0, y = 0;
        get_pos(pos, n, x, y);
        draw_node_label_color(x, y, n->data, n->color);
        if (n->left)
          st.push_back(n->left);
        if (n->right)
          st.push_back(n->right);
      }
    } else
      printxy(dx + 3, dy + 2,
              "Tree is empty. Type digits then [Enter] to insert.");
    ostringstream ss;
    ss << "(W:" << W << " H:" << H << ")";
    printxy(W - (int)ss.str().size() - 2, 0, ss.str());
  }
};

static RBTSceneImpl g_rbt;
Scene *make_rbt_scene() { return (Scene *)&g_rbt; }
