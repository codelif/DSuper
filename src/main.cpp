#include "app.h"
#include "scene.h"
#include "tui.h"

extern Scene *make_menu_scene();

static Scene *g_scene = nullptr;
static bool g_should_quit = false;

void set_scene(Scene *s) { g_scene = s; }
Scene *current_scene() { return g_scene; }
void request_quit() { g_should_quit = true; }
bool should_quit() { return g_should_quit; }

int main() {
  init();
  set_scene(make_menu_scene());

  int c = 0, lc = 0;
  while (!should_quit()) {
    if (current_scene()) {
      current_scene()->render();
      flush_out();
    }
    lc = c;
    c = poll_key();
    if (current_scene())
      current_scene()->on_key(c);
    if (c == 'q' && lc == 'q')
      request_quit();
  }
  deinit();
  return 0;
}
