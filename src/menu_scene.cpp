#include "app.h"
#include "scene.h"
#include "tui.h"
#include <string>
using namespace std;

class MenuScene : public Scene {
public:
  const char *title() const { return "Data Structure Visualizer"; }
  void on_key(int key);
  void render();
};

static const char *items[] = {"BST", "AVL Tree", "Red-Black Tree", "Quit"};
static int sel = 0;

void MenuScene::on_key(int key) {
  if (key == KEY_UP && sel > 0)
    sel--;
  else if (key == KEY_DOWN && sel + 1 < (int)(sizeof(items) / sizeof(items[0])))
    sel++;
  else if (key == '\n' || key == KEY_RIGHT) {
    if (sel == 0)
      set_scene(make_bst_scene());
    else if (sel == 1)
      set_scene(make_avl_scene());
    else if (sel == 2)
      set_scene(make_rbt_scene());
    else
      request_quit();
  } else if (key == 'q') {
    static int last = 0;
    if (last == 'q')
      request_quit();
    last = 'q';
  }
}

void MenuScene::render() {
  Winsize ws = get_term_size();
  clear_scr();
  int W = ws.width, H = ws.height;
  frame(0, 0, W - 1, H - 1);

  string head = string(" ") + title() + " ";
  int hw = (int)head.size() + 2;
  int hx = max(2, (W - hw) / 2);
  frame(hx, 2, hw, 3);
  fill_text(hx + 1, 3, hw - 2, head);

  int mw = 44, mh = (int)(sizeof(items) / sizeof(items[0])) * 3 + 2;
  int mx = max(2, (W - mw) / 2);
  int my = max(6, (H - mh) / 2);
  frame(mx, my, mw, mh);

  for (int i = 0; i < (int)(sizeof(items) / sizeof(items[0])); ++i) {
    int y = my + 1 + i * 3;
    string bullet = (i == sel) ? "▶ " : "  ";
    string line = bullet + string(items[i]);
    fill_text(mx + 2, y, mw - 4, line);
    if (i + 1 < (int)(sizeof(items) / sizeof(items[0])))
      hline(mx + 1, y + 1, mw - 2);
  }

  string foot = "[Enter] Select   [↑/↓] Move   [q q] Quit";
  fill_text(max(2, (W - (int)foot.size()) / 2), H - 2, (int)foot.size(), foot);
}

// small factory so main can get a scene instance
Scene *make_menu_scene() {
  static MenuScene s;
  return &s;
}
