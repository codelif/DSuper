
#include "../app.h"
#include "../scene.h"
#include "../tui.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

class BPlusTree {
public:
  struct Node {
    bool leaf;
    vector<int> keys;
    vector<Node *> children; // internal: size = keys.size() + 1
    Node *next;              // leaf-level linked list

    Node(bool leaf_) : leaf(leaf_), next(nullptr) {}
  };

private:
  Node *root_ = nullptr;
  int t; // "degree" parameter

  int max_keys() const { return 2 * t - 1; }

public:
  BPlusTree(int min_degree = 2) : t(min_degree) {}
  ~BPlusTree() { clear(); }

  Node *root() const { return root_; }

  void clear() {
    if (!root_)
      return;
    vector<Node *> st;
    st.push_back(root_);
    while (!st.empty()) {
      Node *n = st.back();
      st.pop_back();
      for (Node *c : n->children)
        if (c)
          st.push_back(c);
      delete n;
    }
    root_ = nullptr;
  }

  void insert(int k) {
    if (!root_) {
      root_ = new Node(true);
      root_->keys.push_back(k);
      return;
    }

    if ((int)root_->keys.size() == max_keys()) {
      Node *s = new Node(false);
      s->children.push_back(root_);
      split_child(s, 0);
      root_ = s;
    }
    insert_non_full(root_, k);
  }

private:
  void split_child(Node *parent, int idx) {
    Node *child = parent->children[idx];
    Node *new_node = new Node(child->leaf);

    if (child->leaf) {
      int total = (int)child->keys.size();
      int mid = total / 2;

      new_node->keys.assign(child->keys.begin() + mid, child->keys.end());
      child->keys.resize(mid);

      // link leaf level
      new_node->next = child->next;
      child->next = new_node;

      int up_key = new_node->keys.front(); // smallest key in right leaf
      parent->keys.insert(parent->keys.begin() + idx, up_key);
      parent->children.insert(parent->children.begin() + idx + 1, new_node);
    } else {
      int total = (int)child->keys.size();
      int mid = total / 2;

      int up_key = child->keys[mid];

      new_node->keys.assign(child->keys.begin() + mid + 1, child->keys.end());
      child->keys.resize(mid);

      new_node->children.assign(child->children.begin() + mid + 1,
                                child->children.end());
      child->children.resize(mid + 1);

      parent->keys.insert(parent->keys.begin() + idx, up_key);
      parent->children.insert(parent->children.begin() + idx + 1, new_node);
    }
  }

  void insert_non_full(Node *node, int k) {
    if (node->leaf) {
      auto it = lower_bound(node->keys.begin(), node->keys.end(), k);
      node->keys.insert(it, k);
    } else {
      int i = 0;
      while (i < (int)node->keys.size() && k >= node->keys[i])
        ++i;

      if ((int)node->children[i]->keys.size() == max_keys()) {
        split_child(node, i);
        if (k >= node->keys[i])
          ++i;
      }
      insert_non_full(node->children[i], k);
    }
  }
};

struct BPlusTreeScene : public Scene {
  BPlusTree tree;
  string buf;
  vector<string> hist;
  int hist_max = 8;
  vector<int> elements;

  const char *title() const { return "B+ Tree (min degree = 2)"; }

  void push_hist(const string &k) {
    if ((int)hist.size() == hist_max)
      hist.erase(hist.begin());
    hist.push_back(k);
  }

  void rebuild_from_elements() {
    tree.clear();
    for (int v : elements)
      tree.insert(v);
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
      hist.clear();
      elements.clear();
      tree.clear();
      set_scene(make_menu_scene());
      return;
    }

    if (key == 'c') {
      buf.clear();
      hist.clear();
      elements.clear();
      tree.clear();
    } else if (key >= '0' && key <= '9') {
      if (buf.size() < 9)
        buf.push_back((char)key);
    } else if (key == 127 || key == '\b') {
      if (!buf.empty())
        buf.pop_back();
    } else if (key == '\n') {
      if (!buf.empty()) {
        int k = atoi(buf.c_str());
        elements.push_back(k);
        tree.insert(k);
        push_hist(buf + "I");
        buf.clear();
      }
    } else if (key == 'd') {
      if (!buf.empty()) {
        int k = atoi(buf.c_str());
        auto it = find(elements.begin(), elements.end(), k);
        if (it != elements.end()) {
          elements.erase(it);
          rebuild_from_elements();
          push_hist(buf + "D");
        }
        buf.clear();
      }
    } else if (key == 'r') {
      vector<int> sample = {30, 10, 40, 5, 20, 35, 50, 1, 15, 27};
      for (int v : sample) {
        elements.push_back(v);
        tree.insert(v);
      }
      push_hist("sample");
    }
  }

  void draw_node_label_multi(int cx, int cy, const vector<int> &keys) {
    ostringstream ss;
    ss << '[';
    for (size_t i = 0; i < keys.size(); ++i) {
      if (i)
        ss << '|';
      ss << keys[i];
    }
    ss << ']';
    string s = ss.str();
    int x = cx - (int)s.size() / 2;
    printxy(x, cy, s);
  }

  struct LeafPos {
    BPlusTree::Node *n;
    int x;
    int y;
  };

  void draw_tree(BPlusTree::Node *r, int x1, int x2, int y, int y_step,
                 int y_max, vector<LeafPos> &leaves) {
    if (!r)
      return;
    if (x1 > x2)
      return;
    if (y > y_max)
      return;

    int cx = (x1 + x2) / 2;
    draw_node_label_multi(cx, y, r->keys);

    int m = (int)r->children.size();
    if (m == 0) {
      // leaf node: remember its position for leaf-linked-list drawing
      leaves.push_back({r, cx, y});
      return;
    }

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
      int child_cx = (seg_x1 + seg_x2) / 2;
      draw_connector(cx, y, child_cx, next_y);
      draw_tree(r->children[i], seg_x1, seg_x2, next_y, y_step, y_max, leaves);
    }
  }

  void draw_leaf_links(const vector<LeafPos> &leaves) {
    if (leaves.size() < 2)
      return;

    // Sort left-to-right (and top-to-bottom if ever needed)
    vector<LeafPos> v = leaves;
    sort(v.begin(), v.end(), [](const LeafPos &a, const LeafPos &b) {
      if (a.y != b.y)
        return a.y < b.y;
      return a.x < b.x;
    });

    for (size_t i = 0; i + 1 < v.size(); ++i) {
      const auto &L = v[i];
      const auto &R = v[i + 1];

      // Recompute the visual width of the leaf labels
      // so we don't overwrite the '[...|...]' boxes.
      const auto &keysL = L.n->keys;
      const auto &keysR = R.n->keys;

      // left label string
      {
        ostringstream ss;
        ss << '[';
        for (size_t j = 0; j < keysL.size(); ++j) {
          if (j)
            ss << '|';
          ss << keysL[j];
        }
        ss << ']';
        string sL = ss.str();
        int startL = L.x - (int)sL.size() / 2;
        int endL = startL + (int)sL.size() - 1; // last character index

        // right label string (only need its left edge)
        ostringstream ss2;
        ss2 << '[';
        for (size_t j = 0; j < keysR.size(); ++j) {
          if (j)
            ss2 << '|';
          ss2 << keysR[j];
        }
        ss2 << ']';
        string sR = ss2.str();
        int startR = R.x - (int)sR.size() / 2;

        int y = L.y; // same line as leaf boxes

        // draw from endL+1 up to startR-1
        int start = endL + 1;
        int end = startR - 1;
        if (end < start)
          continue;

        // if only one cell, just put an arrow
        if (end == start) {
          put_utf8(start, y, "→");
        } else {
          // line then arrow at the last position
          for (int x = start; x < end; ++x)
            put_utf8(x, y, "─");
          put_utf8(end, y, "→");
        }
      }
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
    for (const string &k : hist)
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

    BPlusTree::Node *root = tree.root();
    if (!root) {
      printxy(x_left, y0, "Tree is empty. Type digits then [Enter] to insert.");
    } else {
      vector<LeafPos> leaves;
      draw_tree(root, x_left, x_right, y0, y_step, y_max, leaves);
      draw_leaf_links(leaves);
    }

    ostringstream ss;
    ss << "(W:" << W << " H:" << H << ")";
    printxy(W - (int)ss.str().size() - 2, 0, ss.str());
  }
};

static BPlusTreeScene g_bptree_scene;
Scene *make_bptree_scene() { return &g_bptree_scene; }
