// heap.cpp
#include "../app.h"
#include "../render.h"
#include "../scene.h"

#include <climits>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

struct HeapImpl {
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
    if (heap[p] < heap[i]) {
      swap(p, i);
      heapifyup(p);
    }
  }

  void heapifydown(int i) {
    int left = leftchild(i);
    int right = rightchild(i);
    int largest = i;

    if (left < (int)heap.size() && heap[left] > heap[largest]) {
      largest = left;
    }
    if (right < (int)heap.size() && heap[right] > heap[largest]) {
      largest = right;
    }

    if (largest != i) {
      swap(i, largest);
      heapifydown(largest);
    }
  }

public:
  int getmax() {
    if (!heap.empty()) {
      return heap[0];
    }
    return INT_MIN;
  }

  void insert(int val) {
    heap.push_back(val);
    heapifyup((int)heap.size() - 1);
  }

  int extractmax() {
    if (heap.empty()) {
      return INT_MIN;
    }
    int max = heap[0];
    heap[0] = heap[heap.size() - 1];
    heap.pop_back();
    if (!heap.empty()) {
      heapifydown(0);
    }
    return max;
  }

  void buildmaxheap(vector<int> arr) {
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
    heapifyup(i);
  }

  void deletekey(int i) {
    if (i >= (int)heap.size())
      return;
    increasekey(i, INT_MAX);
    extractmax();
  }

  void rebuild_nodes() {
    nodes.clear();
    nodes.reserve(heap.size());
    for (int i = 0; i < (int)heap.size(); ++i) {
      nodes.push_back(Node{i});
    }
  }

  // Required alias for TreeScene
  using NodeT = Node;
  using Node = Node; // TreeScene will use HeapImpl::Node

  const char *title() const { return "Max Heap"; }

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
    // n->idx is index into heap
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

// single global instance + factory using the common generic scene
static TreeScene<HeapImpl> g_maxheap_scene;
Scene *make_maxheap_scene() { return &g_maxheap_scene; }
