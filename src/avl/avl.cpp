#include "../app.h"
#include "../scene.h"
#include "../tui.h"
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include <iostream>
using namespace std;
class Node {
public:
  int data;
  Node *left;
  Node *right;
  int height;
  Node(int value) : data(value), left(nullptr), right(nullptr), height(1) {}
};

int height(Node *node) {
  if (node == nullptr) {
    return 0;
  }
  return node->height;
}

int balance(Node *node) {
  if (node == nullptr) {
    return 0;
  }
  return height(node->left) - height(node->right);
}

Node *rotateRight(Node *unbalanced) {
  Node *leftNode = unbalanced->left;
  Node *temp = leftNode->right;

  unbalanced->left = temp;
  leftNode->right = unbalanced;

  unbalanced->height =
      1 + max(height(unbalanced->left), height(unbalanced->right));
  leftNode->height = 1 + max(height(leftNode->left), height(leftNode->right));

  return leftNode;
}

Node *rotateLeft(Node *unbalanced) {
  Node *rightNode = unbalanced->right;
  Node *temp = rightNode->left;

  unbalanced->right = temp;
  rightNode->left = unbalanced;

  unbalanced->height =
      1 + max(height(unbalanced->left), height(unbalanced->right));
  rightNode->height =
      1 + max(height(rightNode->left), height(rightNode->right));

  return rightNode;
}

Node *balanceNode(Node *node, int value) {
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

Node *insert(Node *node, int value) {
  if (node == nullptr) {
    return new Node(value);
  }

  if (value > node->data) {
    node->right = insert(node->right, value);
  } else if (value < node->data) {
    node->left = insert(node->left, value);
  }

  node->height = 1 + max(height(node->left), height(node->right));

  return balanceNode(node, value);
}

Node *minValueNode(Node *node) {
  Node *current = node;
  while (current->left != nullptr) {
    current = current->left;
  }
  return current;
}

Node *deleteNode(Node *node, int value) {
  if (node == nullptr) {
    return node;
  }

  if (value < node->data) {
    node->left = deleteNode(node->left, value);
  } else if (value > node->data) {
    node->right = deleteNode(node->right, value);
  } else {
    // Node with only one child or no child
    if (node->left == nullptr || node->right == nullptr) {
      Node *temp = node->left ? node->left : node->right;
      if (temp == nullptr) {
        temp = node;
        node = nullptr;
      } else {
        *node = *temp;
      }
      delete temp;
    } else {
      // Node with two children
      Node *temp = minValueNode(node->right);
      node->data = temp->data;
      node->right = deleteNode(node->right, temp->data);
    }
  }

  if (node == nullptr) {
    return node;
  }

  node->height = 1 + max(height(node->left), height(node->right));
  return balanceNode(node, value);
}

void preOrder(Node *root) {
  if (root != nullptr) {
    cout << root->data << " ";
    preOrder(root->left);
    preOrder(root->right);
  }
}

struct Pos {
  Node *n;
  int x;
  int y;
};

static void set_pos(vector<Pos> &a, Node *n, int x, int y) {
  for (auto &p : a) {
    if (p.n == n) {
      p.x = x;
      p.y = y;
      return;
    }
  }
  a.push_back({n, x, y});
}
static bool get_pos(const vector<Pos> &a, Node *n, int &x, int &y) {
  for (auto &p : a) {
    if (p.n == n) {
      x = p.x;
      y = p.y;
      return true;
    }
  }
  return false;
}
static void compute_layout(Node *r, int x1, int x2, int y, int ys,
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

class AVLSceneImpl : public Scene {
  Node *root = nullptr;
  string buf;
  vector<string> hist;
  int hist_max = 8;

  void push_hist(string k) {
    if ((int)hist.size() == hist_max)
      hist.erase(hist.begin());
    hist.push_back(k);
  }
  void clear_tree() {
    vector<Node *> st;
    if (root)
      st.push_back(root);
    while (!st.empty()) {
      Node *n = st.back();
      st.pop_back();
      if (n->left)
        st.push_back(n->left);
      if (n->right)
        st.push_back(n->right);
      delete n;
    }
    root = nullptr;
  }

public:
  const char *title() const { return "AVL Tree"; }
  ~AVLSceneImpl() { clear_tree(); }

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
        root = insert(root, k);
        push_hist(buf + "I");
        buf.clear();
      }
    } else if (key == 'd') {
      if (!buf.empty()) {
        int k = atoi(buf.c_str());
        root = deleteNode(root, k);
        push_hist(buf + "D");
        buf.clear();
      }

    } else if (key == 'r') {
      int a[] = {30, 20, 40, 10, 25, 35, 50, 5, 15, 27};
      for (int v : a)
        root = insert(root, v);
    }
  }

  void render() {
    Winsize ws = get_term_size();
    int W = ws.width, H = ws.height;
    clear_scr();
    frame(0, 0, W - 1, H - 1);
    string bar = " AVL Tree (insert only) ";
    frame(2, 1, (int)bar.size() + 2, 3);
    printxy(3, 2, bar);

    int cpw = min(48, max(30, W / 3));
    frame(2, 5, cpw, 7);
    printxy(4, 6, string("Input: ") + (buf.empty() ? "_" : buf));
    printxy(4, 7, "[Enter] insert  [d] Delete [r] sample");
    printxy(4, 8, "[b/Esc] back  [c] clear   [q q] quit");

    frame(2, 13, cpw, 5);
    string h = "History: ";
    for (string k : hist)
      h += k + " ";
    printxy(4, 14, h);

    int dx = cpw + 3, dw = W - dx - 3, dy = 5, dh = H - dy - 3;
    frame(dx, dy, dw, dh);

    if (root) {
      vector<Pos> pos;
      int x1 = dx + 3, x2 = dx + dw - 4, y1 = dy + 2, ys = 3;
      compute_layout(root, x1, x2, y1, ys, pos);

      // edges
      vector<Node *> st;
      st.push_back(root);
      while (!st.empty()) {
        Node *n = st.back();
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
      // nodes
      st.clear();
      st.push_back(root);
      while (!st.empty()) {
        Node *n = st.back();
        st.pop_back();
        int x = 0, y = 0;
        get_pos(pos, n, x, y);
        draw_node_label(x, y, n->data);
        if (n->left)
          st.push_back(n->left);
        if (n->right)
          st.push_back(n->right);
      }
    } else {
      printxy(dx + 3, dy + 2,
              "Tree is empty. Type digits then [Enter] to insert.");
    }

    ostringstream ss;
    ss << "(W:" << W << " H:" << H << ")";
    printxy(W - (int)ss.str().size() - 2, 0, ss.str());
  }
};

// single global instance + factory
static AVLSceneImpl g_avl;
Scene *make_avl_scene() { return (Scene *)&g_avl; }
