#ifndef __TBWM_H__
#define __TBWM_H__

#include <stdint.h>

#ifdef I_X11

#include <X11/Xlib.h>
typedef Window i_window_t;

#endif

typedef struct bar_t bar_t;
typedef struct tab_t tab_t;
typedef uint32_t color_t;

struct bar_t {
  int x, width, screen_height, has_start, has_status;
  char status[256];
  
  tab_t *tabs;
  int count, active;
};

struct tab_t {
  char title[256];
  i_window_t data;
  
  int parent;
};

void tbwm_spawn(char **args);
void tbwm_spawn_text(const char *text);

int tbwm_get_start_width(bar_t *bar);
int tbwm_get_status_width(bar_t *bar);
int tbwm_get_tab_width(bar_t *bar);

void tbwm_draw_bar(int bar_index);

void tbwm_add_tab(int bar_index, const char *title, i_window_t data);
tab_t *tbwm_find_tab(int bar_index, i_window_t data);
void tbwm_remove_tab(int bar_index, i_window_t data);

void tbwm_set_tab_title(tab_t *tab, const char *title);

void tbwm_on_click(i_window_t data, int x, int y);

void i_draw_rect(int x, int y, int width, int height, color_t color);
void i_draw_text(const char *text, int x, int y, int max_length, int center, int bold, color_t color);
void i_draw_line(int x1, int y1, int x2, int y2, color_t color);

int i_get_width(void);
int i_get_height(void);

int i_get_text_width(void);
int i_get_text_height(void);

void i_select_tab(bar_t *bar, int index);

int i_init(void);
void i_tick(void);

// config stuff

#define TBWM_BAR_HEIGHT    40
#define TBWM_BORDER_HEIGHT 2

#define TBWM_REGULAR_FONT "-xos4-terminus-medium-r-normal"
#define TBWM_BOLD_FONT    "-xos4-terminus-bold-r-normal"
#define TBWM_FONT_SIZE    "24"

#define TBWM_START_TEXT "start"

#define TBWM_BACK_COLOR          0xFF1C1B22
#define TBWM_BORDER_COLOR        0xFF36343F
#define TBWM_BACK_SELECT_COLOR   0xFF36343F
#define TBWM_BORDER_SELECT_COLOR 0xFF535260
#define TBWM_FORE_COLOR          0xFFD5D4DD
#define TBWM_FORE_SELECT_COLOR   0xFFFBFBFE

extern bar_t tbwm_bars[];

extern int tbwm_active;
extern int tbwm_redraw;

#ifdef TBWM_CONFIG

bar_t tbwm_bars[] = {
  (bar_t){.x = 0, .width = 640, .screen_height = 768, .has_start = 1, .has_status = 1, .status = "a", .tabs = NULL, .count = 0, .active = -1},
  (bar_t){.x = 640, .width = 640, .screen_height = 768, .has_start = 1, .has_status = 0, .tabs = NULL, .count = 0, .active = -1},
};

int tbwm_active = 0;
int tbwm_redraw = 1;

const char *tbwm_autostart[] = {
  "feh --bg-scale /home/segfaultdev/.config/openbox/wallpaper.jpeg",
  "alacritty",
};

const char *tbwm_start_run = "dmenu_run -b -p > -fn xos4\\ Terminus:size=12";

#endif
#endif
