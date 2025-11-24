#include "app.h"
#include "scene.h"
#include "tui.h"

#include <climits>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

struct FibNode {
  int key;
  int degree;
  bool mark;
  FibNode *parent;
  FibNode *child;
  FibNode *left;
  FibNode *right;

  FibNode(int k) {
    key = k;
    degree = 0;
    mark = false;
    parent = child = nullptr;
    left = right = this;
  }
};

class FibonacciHeap {
  FibNode *min_node;
  int n;

  void add_root(FibNode *x) {
    if (!min_node) {
      x->left = x->right = x;
      min_node = x;
    } else {
      x->left = min_node;
      x->right = min_node->right;
      min_node->right->left = x;
      min_node->right = x;
      if (x->key < min_node->key)
        min_node = x;
    }
  }

  void link(FibNode *y, FibNode *x) {
    y->left->right = y->right;
    y->right->left = y->left;
    y->parent = x;
    y->mark = false;
    if (!x->child) {
      y->left = y->right = y;
      x->child = y;
    } else {
      FibNode *c = x->child;
      y->left = c;
      y->right = c->right;
      c->right->left = y;
      c->right = y;
    }
    x->degree++;
  }

  void consolidate() {
    if (!min_node)
      return;
    int D = 0;
    int nn = n;
    while (nn > 0) {
      nn >>= 1;
      D++;
    }
    D += 5;
    vector<FibNode *> A(D, nullptr);

    vector<FibNode *> roots;
    FibNode *w = min_node;
    if (w) {
      do {
        roots.push_back(w);
        w = w->right;
      } while (w != min_node);
    }

    for (FibNode *x : roots) {
      int d = x->degree;
      while (d >= (int)A.size())
        A.push_back(nullptr);
      while (A[d]) {
        FibNode *y = A[d];
        if (x->key > y->key)
          swap(x, y);
        link(y, x);
        A[d] = nullptr;
        d++;
        while (d >= (int)A.size())
          A.push_back(nullptr);
      }
      A[d] = x;
    }

    min_node = nullptr;
    for (FibNode *x : A) {
      if (!x)
        continue;
      x->left = x->right = x;
      x->parent = nullptr;
      if (!min_node) {
        min_node = x;
      } else {
        add_root(x);
      }
    }
  }

public:
  FibonacciHeap() {
    min_node = nullptr;
    n = 0;
  }

  bool isEmpty() const { return min_node == nullptr; }

  void insert(int key) {
    FibNode *x = new FibNode(key);
    add_root(x);
    n++;
  }

  int getMin() const {
    if (!min_node) {
      cout << "Heap is empty, cannot get minimum.\n";
      return INT_MAX;
    }
    return min_node->key;
  }

  int extractMin() {
    if (!min_node) {
      cout << "Heap is empty, cannot extract minimum.\n";
      return INT_MAX;
    }

    FibNode *z = min_node;
    if (z->child) {
      vector<FibNode *> children;
      FibNode *c = z->child;
      do {
        children.push_back(c);
        c = c->right;
      } while (c != z->child);

      for (FibNode *x : children) {
        x->parent = nullptr;
        x->mark = false;
        x->left = x->right = x;
        add_root(x);
      }
    }

    if (z->right == z) {
      min_node = nullptr;
    } else {
      z->left->right = z->right;
      z->right->left = z->left;
      min_node = z->right;
      consolidate();
    }

    int res = z->key;
    delete z;
    n--;
    if (n == 0)
      min_node = nullptr;
    return res;
  }

  void unionWith(FibonacciHeap &other) {
    if (!other.min_node)
      return;
    vector<FibNode *> roots;
    FibNode *x = other.min_node;
    do {
      roots.push_back(x);
      x = x->right;
    } while (x != other.min_node);

    for (FibNode *r : roots) {
      r->left = r->right = r;
      r->parent = nullptr;
      add_root(r);
    }
    n += other.n;
    other.min_node = nullptr;
    other.n = 0;
  }

  FibNode *getMinRoot() const { return min_node; }
};

struct FibonacciHeapScene : public Scene {
  FibonacciHeap heap;
  string buf;
  vector<string> hist;
  int hist_max = 8;

  const char *title() const { return "Fibonacci Heap"; }

  void push_hist(string k) {
    if ((int)hist.size() == hist_max)
      hist.erase(hist.begin());
    hist.push_back(k);
  }

  void on_key(int key) {
    static int last_q = 0;
    if (key == 'q') {
      if (last_q == 'q') {
        request_quit();
        return;
      }
      last_q = 'q';
    } else {
      last_q = 0;
    }

    if (key == KEY_ESC || key == 'b') {
      buf.clear();
      set_scene(make_menu_scene());
      return;
    }
    if (key == 'c') {
      buf.clear();
      hist.clear();
      while (!heap.isEmpty())
        heap.extractMin();
    } else if (key >= '0' && key <= '9') {
      if (buf.size() < 9)
        buf.push_back((char)key);
    } else if (key == 127 || key == '\b') {
      if (!buf.empty())
        buf.pop_back();
    } else if (key == '\n') {
      if (!buf.empty()) {
        int k = atoi(buf.c_str());
        heap.insert(k);
        push_hist(buf + "I");
        buf.clear();
      }
    } else if (key == 'd') {
      if (!buf.empty()) {
        int k = atoi(buf.c_str());
        FibonacciHeap t;
        bool removed = false;
        while (!heap.isEmpty()) {
          int x = heap.extractMin();
          if (!removed && x == k) {
            removed = true;
            continue;
          }
          t.insert(x);
        }
        heap.unionWith(t);
        push_hist(buf + "D");
        buf.clear();
      }
    } else if (key == 'r') {
      vector<int> s = {10, 3, 7, 1, 20, 15, 5, 8, 12, 30};
      for (int v : s)
        heap.insert(v);
    } else if (key == 'x') {
      if (!heap.isEmpty()) {
        int m = heap.extractMin();
        push_hist(to_string(m) + "X");
      }
    }
  }

  void draw_tree(FibNode *r, int x1, int x2, int y, int y_step, int y_max) {
    if (!r)
      return;
    if (x1 > x2)
      return;
    if (y > y_max)
      return;

    int x = (x1 + x2) / 2;
    draw_node_label(x, y, r->key);

    if (!r->child)
      return;

    vector<FibNode *> children;
    FibNode *c = r->child;
    do {
      children.push_back(c);
      c = c->right;
    } while (c != r->child);

    int m = (int)children.size();
    if (m == 0)
      return;

    int width = x2 - x1 + 1;
    int seg = max(1, width / m);
    int next_y = y + y_step;
    if (next_y > y_max)
      return;

    for (int i = 0; i < m; ++i) {
      int seg_x1 = x1 + i * seg;
      int seg_x2 = (i == m - 1) ? x2 : (x1 + (i + 1) * seg - 1);
      if (seg_x1 > seg_x2)
        continue;
      int cx = (seg_x1 + seg_x2) / 2;
      draw_connector(x, y, cx, next_y);
      draw_tree(children[i], seg_x1, seg_x2, next_y, y_step, y_max);
    }
  }

  void render() {
    Winsize ws = get_term_size();
    int W = ws.width, H = ws.height;
    clear_scr();
    frame(0, 0, W - 1, H - 1);

    string bar = string(" ") + title() + " ";
    frame(2, 1, (int)bar.size() + 2, 3);
    printxy(3, 2, bar);

    int cpw = min(48, max(30, W / 3));
    frame(2, 5, cpw, 7);
    printxy(4, 6, string("Input: ") + (buf.empty() ? "_" : buf));
    printxy(4, 7, "[Enter] insert   [d] delete   [r] sample");
    printxy(4, 8, "[b/Esc] back [c] clear [x] extract MIN [q q] quit");

    frame(2, 13, cpw, 5);
    string h = "History: ";
    for (string k : hist)
      h += k + " ";
    printxy(4, 14, h);

    int fx = cpw + 3;
    int fw = W - fx - 3;
    int fy = 5;
    int fh = H - fy - 3;
    frame(fx, fy, fw, fh);

    int x_left = fx + 2;
    int x_right = fx + fw - 3;
    int y0 = fy + 2;
    int y_max = fy + fh - 3;
    int y_step = 3;

    if (x_left > x_right || y0 > y_max) {
      ostringstream ss2;
      ss2 << "(W:" << W << " H:" << H << ")";
      printxy(W - (int)ss2.str().size() - 2, 0, ss2.str());
      return;
    }

    FibNode *root_min = heap.getMinRoot();
    if (!root_min) {
      printxy(x_left, y0, "Heap is empty. Type digits then [Enter] to insert.");
    } else {
      vector<FibNode *> roots;
      FibNode *p = root_min;
      do {
        roots.push_back(p);
        p = p->right;
      } while (p != root_min);

      int nroots = (int)roots.size();
      int width = x_right - x_left + 1;
      int seg = max(1, width / (nroots == 0 ? 1 : nroots));

      for (int i = 0; i < nroots; ++i) {
        int seg_x1 = x_left + i * seg;
        int seg_x2 = (i == nroots - 1) ? x_right : (x_left + (i + 1) * seg - 1);
        if (seg_x1 > seg_x2)
          continue;
        draw_tree(roots[i], seg_x1, seg_x2, y0, y_step, y_max);
      }
    }

    ostringstream ss;
    ss << "(W:" << W << " H:" << H << ")";
    printxy(W - (int)ss.str().size() - 2, 0, ss.str());
  }
};

static FibonacciHeapScene g_fibonacci_scene;
Scene *make_fibonacci_scene() { return &g_fibonacci_scene; }
