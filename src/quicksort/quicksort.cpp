#include "../app.h"
#include "../scene.h"
#include "../tui.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

struct QuickSortScene : public Scene {
  struct Step {
    vector<int> arr;
    int low, high;
    int pivot; // index
    int i, j;  // scan / partition indices
    string info;
  };

  vector<int> input;
  string buf;
  vector<string> hist;
  int hist_max = 8;

  vector<Step> steps;
  int step_idx = 0;
  bool has_steps = false;

  const char *title() const { return "Quick Sort (step-by-step)"; }

  void push_hist(string k) {
    if ((int)hist.size() == hist_max)
      hist.erase(hist.begin());
    hist.push_back(k);
  }

  void record_step(const vector<int> &arr, int low, int high, int pivot, int i,
                   int j, const string &info) {
    Step s;
    s.arr = arr;
    s.low = low;
    s.high = high;
    s.pivot = pivot;
    s.i = i;
    s.j = j;
    s.info = info;
    steps.push_back(s);
  }

  void swap_int(int &a, int &b) {
    int temp = a;
    a = b;
    b = temp;
  }

  // ---- quick sort (instrumented, based on your code) ----
  int partition_rec(vector<int> &arr, int low, int high) {
    int pivot_val = arr[high];
    int j = low;
    record_step(arr, low, high, high, -1, j, "start partition");

    for (int i = low; i < high; i++) {
      if (arr[i] < pivot_val) {
        swap_int(arr[j], arr[i]);
        record_step(arr, low, high, high, i, j, "swap");
        j++;
      } else {
        record_step(arr, low, high, high, i, j, "scan");
      }
    }
    swap_int(arr[j], arr[high]);
    record_step(arr, low, high, j, -1, -1, "pivot placed");
    return j;
  }

  void quicksort_rec(vector<int> &arr, int low, int high) {
    if (low < high) {
      int pi = partition_rec(arr, low, high);
      record_step(arr, low, high, pi, -1, -1, "after partition");
      quicksort_rec(arr, low, pi - 1);
      quicksort_rec(arr, pi + 1, high);
    }
  }

  void compute_steps() {
    steps.clear();
    step_idx = 0;
    has_steps = false;
    if (input.empty())
      return;

    vector<int> a = input;

    Step s0;
    s0.arr = a;
    s0.low = s0.high = s0.pivot = s0.i = s0.j = -1;
    s0.info = "initial";
    steps.push_back(s0);

    quicksort_rec(a, 0, (int)a.size() - 1);
    has_steps = !steps.empty();
  }

  // ---- input handling ----
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
      input.clear();
      steps.clear();
      has_steps = false;
      set_scene(make_menu_scene());
      return;
    }

    if (key == 'c') {
      buf.clear();
      hist.clear();
      input.clear();
      steps.clear();
      has_steps = false;
    } else if (key >= '0' && key <= '9') {
      if (buf.size() < 9)
        buf.push_back((char)key);
    } else if (key == 127 || key == '\b') {
      if (!buf.empty())
        buf.pop_back();
    } else if (key == '\n') {
      if (!buf.empty()) {
        int k = atoi(buf.c_str());
        input.push_back(k);
        buf.clear();
        has_steps = false;
      }
    } else if (key == 's') {
      compute_steps();
      push_hist("sort");
    } else if (key == 'r') {
      input = {1, 4, 67, 21, 3};
      compute_steps();
      push_hist("sample");
    } else if (key == 'n') {
      if (has_steps && step_idx + 1 < (int)steps.size())
        step_idx++;
    } else if (key == 'p') {
      if (has_steps && step_idx > 0)
        step_idx--;
    }
  }

  // ---- drawing ----
  void draw_array_step(const Step &st, int x_left, int x_right, int y0,
                       int y_max) {
    int n = (int)st.arr.size();
    if (n == 0) {
      printxy(x_left, y0, "Array is empty.");
      return;
    }

    int width = x_right - x_left + 1;
    int seg = max(1, width / max(1, n));
    int y_val = y0 + 1;

    for (int idx = 0; idx < n; ++idx) {
      int seg_x1 = x_left + idx * seg;
      int seg_x2 = (idx == n - 1) ? x_right : (x_left + (idx + 1) * seg - 1);
      if (seg_x1 > seg_x2)
        continue;
      int cx = (seg_x1 + seg_x2) / 2;
      draw_node_label(cx, y_val, st.arr[idx]);

      ostringstream ss;
      ss << idx;
      string id_s = ss.str();
      int ix = cx - (int)id_s.size() / 2;
      if (y_val + 2 <= y_max)
        printxy(ix, y_val + 2, id_s);
    }

    int info_y = y_val + 4;
    if (info_y <= y_max) {
      ostringstream info;
      info << "Step " << (step_idx + 1) << "/" << (int)steps.size();
      if (!st.info.empty())
        info << " - " << st.info;
      if (st.low >= 0 && st.high >= 0)
        info << "  [low=" << st.low << ", high=" << st.high << "]";
      if (st.pivot >= 0)
        info << "  pivot=" << st.pivot;
      if (st.i >= 0)
        info << "  i=" << st.i;
      if (st.j >= 0)
        info << "  j=" << st.j;
      string line = info.str();
      int info_w = x_right - x_left + 1;
      fill_text(x_left, info_y, info_w, line);
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
    frame(2, 5, cpw, 8);

    string input_line = string("Input: ") + (buf.empty() ? "_" : buf);
    fill_text(4, 6, cpw - 4, input_line);

    string arrline = "Array: ";
    for (size_t i = 0; i < input.size(); ++i) {
      if (i)
        arrline += " ";
      arrline += to_string(input[i]);
    }
    fill_text(4, 7, cpw - 4, arrline);

    string controls1 = "[Enter] add   [s] sort   [n/p] step";
    string controls2 = "[r] sample   [b/Esc] back   [c] clear   [q q] quit";
    fill_text(4, 8, cpw - 4, controls1);
    fill_text(4, 9, cpw - 4, controls2);

    frame(2, 14, cpw, 5);
    string h = "History: ";
    for (string k : hist)
      h += k + " ";
    fill_text(4, 15, cpw - 4, h);

    int fx = cpw + 3;
    int fw = W - fx - 3;
    int fy = 5;
    int fh = H - fy - 3;
    frame(fx, fy, fw, fh);

    int x_left = fx + 2;
    int x_right = fx + fw - 3;
    int y0 = fy + 2;
    int y_max = fy + fh - 3;

    if (x_left > x_right || y0 > y_max) {
      ostringstream ss2;
      ss2 << "(W:" << W << " H:" << H << ")";
      printxy(W - (int)ss2.str().size() - 2, 0, ss2.str());
      return;
    }

    if (!has_steps || steps.empty()) {
      string msg =
          "Type numbers + [Enter], then press [s] to run quick sort.";
      int w = x_right - x_left + 1;
      fill_text(x_left, y0, w, msg);
    } else {
      const Step &st = steps[step_idx];
      draw_array_step(st, x_left, x_right, y0, y_max);
    }

    ostringstream ss;
    ss << "(W:" << W << " H:" << H << ")";
    printxy(W - (int)ss.str().size() - 2, 0, ss.str());
  }
};

static QuickSortScene g_quicksort_scene;
Scene *make_quicksort_scene() { return &g_quicksort_scene; }

