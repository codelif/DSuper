// tree_scene.h
#pragma once
#include "app.h"
#include "scene.h"
#include "tui.h"
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

template <class Impl> class TreeScene : public Scene {
  using Node = typename Impl::Node;

  Impl impl;
  std::string buf;
  std::vector<std::string> hist;
  int hist_max = 8;

  struct Pos {
    Node *n;
    int x, y;
  };

  static void set_pos(std::vector<Pos> &a, Node *n, int x, int y) {
    for (auto &p : a)
      if (p.n == n) {
        p.x = x;
        p.y = y;
        return;
      }
    a.push_back({n, x, y});
  }
  static bool get_pos(const std::vector<Pos> &a, Node *n, int &x, int &y) {
    for (auto &p : a)
      if (p.n == n) {
        x = p.x;
        y = p.y;
        return true;
      }
    return false;
  }
  static void compute_layout(Impl &impl, Node *r, int x1, int x2, int y, int ys,
                             std::vector<Pos> &pos) {
    if (!r)
      return;
    int mid = (x1 + x2) / 2;
    set_pos(pos, r, mid, y);
    if (auto *L = impl.left(r))
      compute_layout(impl, L, x1, mid - 4, y + ys, ys, pos);
    if (auto *R = impl.right(r))
      compute_layout(impl, R, mid + 4, x2, y + ys, ys, pos);
  }

  void push_hist(string k) {
    if ((int)hist.size() == hist_max)
      hist.erase(hist.begin());
    hist.push_back(k);
  }

public:
  ~TreeScene() { impl.clear(); }
  const char *title() const { return impl.title(); }

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
      impl.clear();
      hist.clear();
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
        impl.insert(k);
        push_hist(buf + "I");
        buf.clear();
      }
    } else if (key == 'd') {
      if (!buf.empty()) {
        int k = atoi(buf.c_str());
        impl.erase(k);
        push_hist(buf + "D");
        buf.clear();
      }
    } else if (key == 'r') {
      for (int v : impl.sample())
        impl.insert(v);
    }
  }

  void render() {
    Winsize ws = get_term_size();
    int W = ws.width, H = ws.height;
    clear_scr();
    frame(0, 0, W - 1, H - 1);

    std::string bar = std::string(" ") + impl.title() + " (insert only) ";
    frame(2, 1, (int)bar.size() + 2, 3);
    printxy(3, 2, bar);

    int cpw = std::min(48, std::max(30, W / 3));
    frame(2, 5, cpw, 7);
    printxy(4, 6, std::string("Input: ") + (buf.empty() ? "_" : buf));
    printxy(4, 7, "[Enter] insert   [d] delete   [r] sample");
    printxy(4, 8, "[b/Esc] back   [c] clear   [q q] quit");

    frame(2, 13, cpw, 5);
    std::string h = "History: ";
    for (string k : hist)
      h += k + " ";
    printxy(4, 14, h);

    int dx = cpw + 3, dw = W - dx - 3, dy = 5, dh = H - dy - 3;
    frame(dx, dy, dw, dh);

    if (auto *root = impl.root()) {
      std::vector<Pos> pos;
      int x1 = dx + 3, x2 = dx + dw - 4, y1 = dy + 2, ys = 3;
      compute_layout(impl, root, x1, x2, y1, ys, pos);

      // edges
      std::vector<Node *> st;
      st.push_back(root);
      while (!st.empty()) {
        Node *n = st.back();
        st.pop_back();
        int x = 0, y = 0;
        get_pos(pos, n, x, y);
        if (auto *L = impl.left(n)) {
          int xl = 0, yl = 0;
          get_pos(pos, L, xl, yl);
          draw_connector(x, y, xl, yl);
          st.push_back(L);
        }
        if (auto *R = impl.right(n)) {
          int xr = 0, yr = 0;
          get_pos(pos, R, xr, yr);
          draw_connector(x, y, xr, yr);
          st.push_back(R);
        }
      }

      // nodes
      st.clear();
      st.push_back(root);
      while (!st.empty()) {
        Node *n = st.back();
        st.pop_back();
        int x = 0, y = 0;
        get_pos(pos, n, x, y);
        impl.draw_label(x, y, n); // per-tree customization
        if (auto *L = impl.left(n))
          st.push_back(L);
        if (auto *R = impl.right(n))
          st.push_back(R);
      }
    } else {
      printxy(dx + 3, dy + 2,
              "Tree is empty. Type digits then [Enter] to insert.");
    }

    std::ostringstream ss;
    ss << "(W:" << W << " H:" << H << ")";
    printxy(W - (int)ss.str().size() - 2, 0, ss.str());
  }
};
