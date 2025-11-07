#include "tui.h"
#include <csignal>
#include <iostream>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
using namespace std;

#define CSI "\x1b["
static struct termios og_termios;

int KEY_UP = 128;
int KEY_DOWN = 128 + 1;
int KEY_RIGHT = 128 + 2;
int KEY_LEFT = 128 + 3;
int KEY_ESC = 27;

static void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &og_termios);
  struct termios raw = og_termios;
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
static void disable_raw_mode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &og_termios);
}

void decset(int code) { cout << CSI << '?' << code << 'h'; }
void decrst(int code) { cout << CSI << '?' << code << 'l'; }

void clear_scr() { cout << CSI << "2J" << CSI << "H"; }

Winsize get_term_size() {
  struct winsize ws;
  ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
  return Winsize{(int)ws.ws_row, (int)ws.ws_col};
}

void deinit() {
  signal(SIGINT, SIG_DFL);
  decset(25);
  decrst(1049);
  disable_raw_mode();
  cout << flush;
}

void init() {
  enable_raw_mode();
  decrst(25);
  decset(1049);
  cout << flush;
  signal(SIGINT, SIG_IGN);
  atexit(deinit);
}

void printxy(int x, int y, const string &s) {
  cout << CSI << (y + 1) << ';' << (x + 1) << 'H' << s;
}

void put_utf8(int x, int y, const char *s) {
  cout << CSI << (y + 1) << ';' << (x + 1) << 'H' << s;
}

void flush_out() { cout << std::flush; }

static int read_byte_with_timeout(char *out, int timeout_ms) {
  fd_set set;
  FD_ZERO(&set);
  FD_SET(STDIN_FILENO, &set);
  struct timeval tv;
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;
  int rv = select(STDIN_FILENO + 1, &set, nullptr, nullptr, &tv);
  if (rv > 0)
    return (read(STDIN_FILENO, out, 1) == 1) ? 1 : 0;
  return 0;
}

int poll_key() {
  unsigned char c;
  for (;;) {
    if (read(STDIN_FILENO, &c, 1) != 1)
      continue;
    if (c != '\x1b') {
      if (c == '\r' || c == '\n')
        return '\n';
      return c;
    }
    unsigned char c1;
    if (!read_byte_with_timeout((char *)&c1, 50))
      return KEY_ESC;
    if (c1 == '[' || c1 == 'O') {
      unsigned char c2;
      if (!read(STDIN_FILENO, &c2, 1))
        continue;
      switch (c2) {
      case 'A':
        return KEY_UP;
      case 'B':
        return KEY_DOWN;
      case 'C':
        return KEY_RIGHT;
      case 'D':
        return KEY_LEFT;
      default:
        while (read(STDIN_FILENO, (char *)&c2, 1) == 1) {
          if (c2 >= 0x40 && c2 <= 0x7E)
            break;
        }
        continue;
      }
    }
    while (read(STDIN_FILENO, (char *)&c1, 1) == 1) {
      if (c1 >= 0x40 && c1 <= 0x7E)
        break;
    }
  }
}
