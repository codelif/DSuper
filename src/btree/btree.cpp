#include "../app.h"
#include "../scene.h"
#include "../tui.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

class BTree {
public:
  struct Node {
    bool leaf;
    vector<int> keys;
    vector<Node *> children; // size = keys.size() + 1 if !leaf

    Node(bool leaf_) : leaf(leaf_) {}
  };

private:
  Node *root_ = nullptr;
  int t; // minimum degree

public:
  BTree(int min_degree = 2) : t(min_degree) {}
  ~BTree() { clear(); }

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

    if ((int)root_->keys.size() == 2 * t - 1) {
      Node *s = new Node(false);
      s->children.push_back(root_);
      split_child(s, 0);
      root_ = s;
    }
    insert_non_full(root_, k);
  }

private:
  void split_child(Node *x, int i) {
    Node *y = x->children[i];
    Node *z = new Node(y->leaf);

    int mid_index = t - 1;
    int mid_key = y->keys[mid_index];

    // z gets keys [t .. 2t-2]
    z->keys.assign(y->keys.begin() + t, y->keys.end());
    y->keys.resize(mid_index);

    if (!y->leaf) {
      // children: size = 2t; move [t .. 2t-1] into z
      z->children.assign(y->children.begin() + t, y->children.end());
      y->children.resize(t);
    }

    x->children.insert(x->children.begin() + i + 1, z);
    x->keys.insert(x->keys.begin() + i, mid_key);
  }

  void insert_non_full(Node *x, int k) {
    int i = (int)x->keys.size() - 1;
    if (x->leaf) {
      x->keys.push_back(0);
      while (i >= 0 && k < x->keys[i]) {
        x->keys[i + 1] = x->keys[i];
        --i;
      }
      x->keys[i + 1] = k;
    } else {
      while (i >= 0 && k < x->keys[i])
        --i;
      ++i;
      if ((int)x->children[i]->keys.size() == 2 * t - 1) {
        split_child(x, i);
        if (k > x->keys[i])
          ++i;
      }
      insert_non_full(x->children[i], k);
    }
  }
};

struct BTreeScene : public Scene {
  BTree tree;
  string buf;
  vector<string> hist;
  int hist_max = 8;
  vector<int> elements; // maintains current set of values

  const char *title() const { return "B-Tree (min degree = 2)"; }

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
        // avoid duplicate insertions into the multiset if you want
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

  void draw_tree(BTree::Node *r, int x1, int x2, int y, int y_step, int y_max) {
    if (!r)
      return;
    if (x1 > x2)
      return;
    if (y > y_max)
      return;

    int cx = (x1 + x2) / 2;
    draw_node_label_multi(cx, y, r->keys);

    int m = (int)r->children.size();
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
      int child_cx = (seg_x1 + seg_x2) / 2;
      draw_connector(cx, y, child_cx, next_y);
      draw_tree(r->children[i], seg_x1, seg_x2, next_y, y_step, y_max);
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

    BTree::Node *root = tree.root();
    if (!root) {
      printxy(x_left, y0,
              "Tree is empty. Type digits then [Enter] to insert.");
    } else {
      draw_tree(root, x_left, x_right, y0, y_step, y_max);
    }

    ostringstream ss;
    ss << "(W:" << W << " H:" << H << ")";
    printxy(W - (int)ss.str().size() - 2, 0, ss.str());
  }
};

static BTreeScene g_btree_scene;
Scene *make_btree_scene() { return &g_btree_scene; }

