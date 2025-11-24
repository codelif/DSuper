// minheap.cpp
#include "../app.h"
#include "../render.h"
#include "../scene.h"

#include <climits>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

struct MinHeapImpl {
  struct Node {
    int idx;
  };

  vector<int> heap;
  vector<Node> nodes;

  int parent(int i) { return (i - 1) / 2; }
  int leftchild(int i) { return 2 * i + 1; }
  int rightchild(int i) { return 2 * i + 2; }

  void swap(int i, int j) {
    int temp = heap[j];
    heap[j] = heap[i];
    heap[i] = temp;
  }

  void heapifyup(int i) {
    if (i == 0)
      return;
    int p = parent(i);

    if (heap[p] > heap[i]) {
      swap(p, i);
      heapifyup(p);
    }
  }

  void heapifydown(int i) {
    int left = leftchild(i);
    int right = rightchild(i);
    int smallest = i;

    if (left < (int)heap.size() && heap[left] < heap[smallest]) {
      smallest = left;
    }
    if (right < (int)heap.size() && heap[right] < heap[smallest]) {
      smallest = right;
    }

    if (smallest != i) {
      swap(i, smallest);
      heapifydown(smallest);
    }
  }

public:
  int getmin() {
    if (!heap.empty()) {
      return heap[0];
    }
    return INT_MAX;
  }

  void insert(int val) {
    heap.push_back(val);
    heapifyup((int)heap.size() - 1);
  }

  int extractmin() {
    if (heap.empty()) {
      return INT_MAX;
    }
    int mn = heap[0];
    heap[0] = heap[heap.size() - 1];
    heap.pop_back();
    if (!heap.empty()) {
      heapifydown(0);
    }
    return mn;
  }

  void buildminheap(vector<int> arr) {
    heap = arr;
    for (int i = (int)(heap.size() / 2) - 1; i >= 0; i--) {
      heapifydown(i);
    }
  }

  void increasekey(int i, int val) {
    if (i >= (int)heap.size() || val < heap[i]) {
      return;
    }
    heap[i] = val;
    heapifydown(i);
  }

  void deletekey(int i) {
    if (i >= (int)heap.size())
      return;
    heap[i] = INT_MIN;
    heapifyup(i);
    extractmin();
  }

  void rebuild_nodes() {
    nodes.clear();
    nodes.reserve(heap.size());
    for (int i = 0; i < (int)heap.size(); ++i) {
      nodes.push_back(Node{i});
    }
  }

  using NodeT = Node;
  using NodeType = Node;

  const char *title() const { return "Min Heap"; }

  Node *root() {
    if (heap.empty())
      return nullptr;
    rebuild_nodes();
    return &nodes[0];
  }

  Node *left(Node *n) {
    if (!n)
      return nullptr;
    int li = leftchild(n->idx);
    if (li >= (int)heap.size())
      return nullptr;
    return &nodes[li];
  }

  Node *right(Node *n) {
    if (!n)
      return nullptr;
    int ri = rightchild(n->idx);
    if (ri >= (int)heap.size())
      return nullptr;
    return &nodes[ri];
  }

  void draw_label(int x, int y, Node *n) const {
    int val = heap[n->idx];
    draw_node_label(x, y, val);
  }

  void erase(int val) {
    for (int i = 0; i < (int)heap.size(); ++i) {
      if (heap[i] == val) {
        deletekey(i);
        break;
      }
    }
  }

  void clear() {
    heap.clear();
    nodes.clear();
  }

  vector<int> sample() const { return {30, 10, 40, 5, 20, 35, 50, 1, 15, 27}; }
};

// global instance
static TreeScene<MinHeapImpl> g_minheap_scene;
Scene *make_minheap_scene() { return &g_minheap_scene; }
