#include "tui.h"
#include <algorithm>
#include <sstream>
using namespace std;

void hline(int x, int y, int w) {
  if (w <= 0)
    return;
  for (int i = 0; i < w; ++i)
    put_utf8(x + i, y, "─");
}
void vline(int x, int y, int h) {
  if (h <= 0)
    return;
  for (int i = 0; i < h; ++i)
    put_utf8(x, y + i, "│");
}
void frame(int x, int y, int w, int h) {
  if (w < 2 || h < 2)
    return;
  put_utf8(x, y, "┌");
  put_utf8(x + w - 1, y, "┐");
  put_utf8(x, y + h - 1, "└");
  put_utf8(x + w - 1, y + h - 1, "┘");
  hline(x + 1, y, w - 2);
  hline(x + 1, y + h - 1, w - 2);
  vline(x, y + 1, h - 2);
  vline(x + w - 1, y + 1, h - 2);
}
void fill_text(int x, int y, int w, const string &s) {
  string t = s.substr(0, max(0, w));
  printxy(x, y, t);
}

void draw_connector(int x1, int y1, int x2, int y2) {
  if (y2 <= y1)
    return;
  int hy = y1 + 1;
  if (x2 == x1) {
    vline(x1, hy, y2 - hy);
    return;
  }
  if (x2 < x1)
    put_utf8(x1, hy, "└");
  else
    put_utf8(x1, hy, "┘");
  int dx = abs(x2 - x1) - 1;
  if (dx > 0)
    for (int i = 0; i < dx; ++i)
      put_utf8(min(x1, x2) + 1 + i, hy, "─");
  put_utf8(x2, hy, (x2 < x1) ? "┐" : "┌");
  vline(x2, hy + 1, y2 - (hy + 1));
}

void draw_node_label(int cx, int cy, int key) {
  ostringstream ss;
  ss << '[' << key << ']';
  string s = ss.str();
  int x = cx - (int)s.size() / 2;
  printxy(x, cy, s);
}
