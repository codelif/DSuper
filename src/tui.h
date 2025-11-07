#include <string>

struct Winsize {
  int height, width;
};

void init();
void deinit();

Winsize get_term_size();
int poll_key();

extern int KEY_UP, KEY_DOWN, KEY_RIGHT, KEY_LEFT, KEY_ESC;

void printxy(int x, int y, const std::string &s);
void put_utf8(int x, int y, const char *s);
void clear_scr();
void flush_out();

void decset(int code);
void decrst(int code);

// ---- drawing helpers (moved here from draw.h) ----
void hline(int x, int y, int w);
void vline(int x, int y, int h);
void frame(int x, int y, int w, int h);
void fill_text(int x, int y, int w, const std::string &s);

void draw_connector(int x1, int y1, int x2, int y2);
void draw_node_label(int cx, int cy, int key);
