#include "app.h"
#include "scene.h"
#include "tui.h"

#include <climits>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

struct Node {
  int key;
  int degree;
  Node *parent;
  Node *child;
  Node *sibling;

  Node(int k) {
    key = k;
    degree = 0;
    parent = child = sibling = nullptr;
  }
};

class BinomialHeap {
private:
  Node *head;

  static Node *mergeRootLists(Node *h1, Node *h2) {
    if (!h1)
      return h2;
    if (!h2)
      return h1;

    Node *head = nullptr;
    Node *tail = nullptr;

    Node *a = h1;
    Node *b = h2;

    if (a->degree <= b->degree) {
      head = a;
      a = a->sibling;
    } else {
      head = b;
      b = b->sibling;
    }
    tail = head;

    while (a && b) {
      if (a->degree <= b->degree) {
        tail->sibling = a;
        a = a->sibling;
      } else {
        tail->sibling = b;
        b = b->sibling;
      }
      tail = tail->sibling;
    }

    tail->sibling = (a ? a : b);

    return head;
  }

  static void linkTrees(Node *y, Node *z) {
    y->parent = z;
    y->sibling = z->child;
    z->child = y;
    z->degree++;
  }

  static Node *unionHeaps(Node *h1, Node *h2) {
    Node *newHead = mergeRootLists(h1, h2);
    if (!newHead)
      return nullptr;

    Node *prev = nullptr;
    Node *curr = newHead;
    Node *next = curr->sibling;

    while (next != nullptr) {
      if ((curr->degree != next->degree) ||
          (next->sibling != nullptr && next->degree == next->sibling->degree)) {
        prev = curr;
        curr = next;
      } else {
        if (curr->key <= next->key) {
          curr->sibling = next->sibling;
          linkTrees(next, curr);
        } else {
          if (prev == nullptr) {
            newHead = next;
          } else {
            prev->sibling = next;
          }
          linkTrees(curr, next);
          curr = next;
        }
      }
      next = curr->sibling;
    }

    return newHead;
  }

public:
  BinomialHeap() { head = nullptr; }

  bool isEmpty() const { return head == nullptr; }

  void insert(int key) {
    Node *newNode = new Node(key);
    head = unionHeaps(head, newNode);
  }

  int getMin() const {
    if (isEmpty()) {
      cout << "Heap is empty, cannot get minimum.\n";
      return INT_MAX;
    }

    int minVal = INT_MAX;
    Node *curr = head;
    while (curr != nullptr) {
      if (curr->key < minVal) {
        minVal = curr->key;
      }
      curr = curr->sibling;
    }
    return minVal;
  }

  int extractMin() {
    if (isEmpty()) {
      cout << "Heap is empty, cannot extract minimum.\n";
      return INT_MAX;
    }

    Node *minNode = head;
    Node *minPrev = nullptr;

    Node *curr = head->sibling;
    Node *prev = head;

    int minVal = head->key;

    while (curr != nullptr) {
      if (curr->key < minVal) {
        minVal = curr->key;
        minNode = curr;
        minPrev = prev;
      }
      prev = curr;
      curr = curr->sibling;
    }

    if (minPrev != nullptr) {
      minPrev->sibling = minNode->sibling;
    } else {
      head = minNode->sibling;
    }

    Node *child = minNode->child;

    Node *prevChild = nullptr;
    while (child != nullptr) {
      Node *nextChild = child->sibling;
      child->sibling = prevChild;
      child->parent = nullptr;
      prevChild = child;
      child = nextChild;
    }

    head = unionHeaps(head, prevChild);

    int result = minNode->key;
    delete minNode;

    return result;
  }

  void unionWith(BinomialHeap &other) {
    head = unionHeaps(this->head, other.head);
    other.head = nullptr;
  }

  void printHeap() const {
    cout << "Root list: ";
    Node *curr = head;
    while (curr != nullptr) {
      cout << "(key=" << curr->key << ",deg=" << curr->degree << ") ";
      curr = curr->sibling;
    }
    cout << "\n";
  }

  Node *getHead() const { return head; }
};

struct BinomialHeapScene : public Scene {
  BinomialHeap heap;
  string buf;
  vector<string> hist;
  int hist_max = 8;

  const char *title() const { return "Binomial Heap"; }

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
        BinomialHeap t;
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
      vector<int> s = {10, 3, 7, 1, 20, 15, 5, 8};
      for (int v : s)
        heap.insert(v);
    }
  }

  void draw_tree(Node *r, int x1, int x2, int y, int y_step, int y_max) {
    if (!r)
      return;
    if (x1 > x2)
      return;
    if (y > y_max)
      return;

    int x = (x1 + x2) / 2;
    draw_node_label(x, y, r->key);

    Node *c = r->child;
    if (!c)
      return;

    vector<Node *> children;
    for (Node *t = c; t; t = t->sibling)
      children.push_back(t);
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
    printxy(4, 8, "[b/Esc] back   [c] clear   [q q] quit");

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
      ostringstream ss;
      ss << "(W:" << W << " H:" << H << ")";
      printxy(W - (int)ss.str().size() - 2, 0, ss.str());
      return;
    }

    Node *root_head = heap.getHead();
    if (!root_head) {
      printxy(x_left, y0, "Heap is empty. Type digits then [Enter] to insert.");
    } else {
      vector<Node *> roots;
      for (Node *p = root_head; p; p = p->sibling)
        roots.push_back(p);
      int n = (int)roots.size();
      int width = x_right - x_left + 1;
      int seg = max(1, width / (n == 0 ? 1 : n));

      for (int i = 0; i < n; ++i) {
        int seg_x1 = x_left + i * seg;
        int seg_x2 = (i == n - 1) ? x_right : (x_left + (i + 1) * seg - 1);
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

static BinomialHeapScene g_binomial_scene;
Scene *make_binomial_scene() { return &g_binomial_scene; }
