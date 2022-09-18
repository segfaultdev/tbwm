#define TBWM_CONFIG

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <tbwm.h>

void tbwm_spawn(char **args) {
  if (!fork()) {
    setsid();
    execvp(args[0], args);
    exit(0);
  }
}

void tbwm_spawn_text(const char *text) {
  char **args = NULL;
  int count = 0;
  
  char *buffer = NULL;
  int length = 0;
  
  for (;;) {
    if (*text <= 32) {
      buffer = realloc(buffer, length + 1);
      buffer[length] = '\0';
      
      args = realloc(args, (count + 1) * sizeof(char *));
      args[count++] = buffer;
      
      buffer = NULL;
      length = 0;
    } else if (*text == '\\') {
      text++;
      
      buffer = realloc(buffer, length + 1);
      buffer[length++] = *text;
    } else {
      buffer = realloc(buffer, length + 1);
      buffer[length++] = *text;
    }
    
    if (!(*text)) break;
    text++;
  }
  
  args = realloc(args, (count + 1) * sizeof(char *));
  args[count] = NULL;
  
  tbwm_spawn(args);
}

int tbwm_get_start_width(bar_t *bar) {
  if (bar->has_start) {
    return strlen(TBWM_START_TEXT) * i_get_text_width() + 2 * 8 + 2;
  } else {
    return 0;
  }
}

int tbwm_get_status_width(bar_t *bar) {
  if (bar->has_status) {
    return strlen(bar->status) * i_get_text_width() + 2 * 8 + 2;
  } else {
    return 0;
  }
}

int tbwm_get_tab_width(bar_t *bar) {
  return bar->width - (tbwm_get_start_width(bar) + tbwm_get_status_width(bar));
}

void tbwm_draw_bar(int bar_index) {
  bar_t *bar = tbwm_bars + bar_index;
  
  i_draw_rect(bar->x, 0, bar->width, TBWM_BAR_HEIGHT - TBWM_BORDER_HEIGHT, TBWM_BACK_COLOR);
  i_draw_rect(bar->x, TBWM_BAR_HEIGHT - TBWM_BORDER_HEIGHT, bar->width, TBWM_BORDER_HEIGHT, TBWM_BORDER_COLOR);
  
  int tabs_width = tbwm_get_tab_width(bar);
  int x = bar->x + tbwm_get_start_width(bar);
  
  if (bar->has_start) {
    i_draw_rect(x - 2, 0, 2, TBWM_BAR_HEIGHT, TBWM_BORDER_SELECT_COLOR);
    i_draw_text(TBWM_START_TEXT, bar->x + 10, (TBWM_BAR_HEIGHT - i_get_text_height()) / 2, 0, 0, 1, TBWM_FORE_COLOR);
  }
  
  if (bar->has_status) {
    i_draw_rect(x + tabs_width, 0, 2, TBWM_BAR_HEIGHT, TBWM_BORDER_SELECT_COLOR);
    i_draw_text(bar->status, x + tabs_width + 8, (TBWM_BAR_HEIGHT - i_get_text_height()) / 2, 0, 0, 0, TBWM_FORE_COLOR);
  }
  
  for (int i = -1; i < bar->count; i++) {
    int width = (((i + 2) * tabs_width) / (bar->count + 1)) - (((i + 1) * tabs_width) / (bar->count + 1));
    int end_x = x + width;
    
    color_t text_color = TBWM_FORE_COLOR;
    int bold = 0;
    
    if (bar->active == i) {
      i_draw_rect(x, 0, width, TBWM_BAR_HEIGHT - TBWM_BORDER_HEIGHT, TBWM_BACK_SELECT_COLOR);
      i_draw_rect(x, TBWM_BAR_HEIGHT - TBWM_BORDER_HEIGHT, width, TBWM_BORDER_HEIGHT, TBWM_BORDER_SELECT_COLOR);
      
      text_color = TBWM_FORE_SELECT_COLOR;
      bold = 1;
    }
    
    if (i >= 0) {
      int start_y = (TBWM_BAR_HEIGHT - 10) / 2;
      end_x -= 18;
      
      i_draw_line(end_x, start_y, end_x + 9, start_y + 9, text_color);
      i_draw_line(end_x, start_y + 9, end_x + 9, start_y, text_color);
      
      if (bar->active == i && bar_index < (sizeof(tbwm_bars) / sizeof(bar_t)) - 1) {
        end_x -= 18;
        
        i_draw_line(end_x, start_y, end_x + 4, start_y + 4, text_color);
        i_draw_line(end_x, start_y + 9, end_x + 4, start_y + 5, text_color);
        i_draw_line(end_x + 5, start_y, end_x + 9, start_y + 4, text_color);
        i_draw_line(end_x + 5, start_y + 9, end_x + 9, start_y + 5, text_color);
      }
      
      if (bar->active == i && bar_index > 0) {
        end_x -= 18;
        
        i_draw_line(end_x + 4, start_y, end_x, start_y + 4, text_color);
        i_draw_line(end_x + 4, start_y + 9, end_x, start_y + 5, text_color);
        i_draw_line(end_x + 9, start_y, end_x + 5, start_y + 4, text_color);
        i_draw_line(end_x + 9, start_y + 9, end_x + 5, start_y + 5, text_color);
      }
    }
    
    int text_width = (end_x - x) - 16;
    
    if (i >= 0) {
      i_draw_text(bar->tabs[i].title, x + 8, (TBWM_BAR_HEIGHT - i_get_text_height()) / 2, 0, 0, bold, text_color);
    } else {
      char buffer[64];
      
      sprintf(buffer, "(screen %d)", bar_index + 1);
      i_draw_text(buffer, x + 8, (TBWM_BAR_HEIGHT - i_get_text_height()) / 2, 0, 0, bold, text_color);
    }
    
    x += width;
  }
}

void tbwm_add_tab(int bar_index, const char *title, i_window_t data) {
  bar_t *bar = tbwm_bars + bar_index;
  
  bar->tabs = realloc(bar->tabs, (bar->count + 1) * sizeof(tab_t));
  strcpy(bar->tabs[bar->count].title, title);
  
  bar->tabs[bar->count].data = data;
  bar->tabs[bar->count].parent = bar_index;
  
  i_select_tab(bar, bar->count);
  bar->count++;
  
  tbwm_redraw = 1;
}

tab_t *tbwm_find_tab(int bar_index, i_window_t data) {
  if (bar_index >= 0) {
    for (int i = 0; i < tbwm_bars[bar_index].count; i++) {
      if (!memcmp(&data, &(tbwm_bars[bar_index].tabs[i].data), sizeof(i_window_t))) {
        return tbwm_bars[bar_index].tabs + i;
      }
    }
    
    return NULL;
  }
  
  for (int i = 0; i < sizeof(tbwm_bars) / sizeof(bar_t); i++) {
    for (int j = 0; j < tbwm_bars[i].count; j++) {
      if (!memcmp(&data, &(tbwm_bars[i].tabs[j].data), sizeof(i_window_t))) {
        return tbwm_bars[i].tabs + j;
      }
    }
  }
  
  return NULL;
}

void tbwm_remove_tab(int bar_index, i_window_t data) {
  tab_t *tab = tbwm_find_tab(bar_index, data);
  if (!tab) return;
  
  bar_t *bar = tbwm_bars + tab->parent;
  int index = (tab - bar->tabs) / sizeof(tab_t);
  
  bar->count--;
  
  memmove(bar->tabs + index, bar->tabs + index + 1, bar->count - index);
  
  if (bar->count) {
    bar->tabs = realloc(bar->tabs, bar->count * sizeof(tab_t));
  } else {
    free(bar->tabs);
    bar->tabs = NULL;
  }
  
  if (bar->active == index) {
    if (bar->active >= bar->count) bar->active = bar->count - 1;
    i_select_tab(bar, bar->active);
  } else if (bar->active > index) {
    bar->active--;
  }
  
  tbwm_redraw = 1;
}

void tbwm_set_tab_title(tab_t *tab, const char *title) {
  if (!strcmp(tab->title, title)) return;
  
  strcpy(tab->title, title);
  tbwm_redraw = 1;
}

void tbwm_on_click(i_window_t data, int x, int y) {
  tab_t *tab = tbwm_find_tab(-1, data);
  
  for (int i = 0; i < sizeof(tbwm_bars) / sizeof(bar_t); i++) {
    if (x >= tbwm_bars[i].x && x < tbwm_bars[i].x + tbwm_bars[i].width) {
      tbwm_active = i;
      break;
    }
  }
  
  bar_t *bar = tbwm_bars + tbwm_active;
  
  if (!tab && y < TBWM_BAR_HEIGHT) {
    int tabs_width = tbwm_get_tab_width(bar);
    int tab_x = bar->x;
    
    if (x >= tab_x && x < tab_x + tbwm_get_start_width(bar)) {
      tbwm_spawn_text(tbwm_start_run);
    }
    
    tab_x += tbwm_get_start_width(bar);
    
    for (int i = -1; i < bar->count; i++) {
      int width = (((i + 2) * tabs_width) / (bar->count + 1)) - (((i + 1) * tabs_width) / (bar->count + 1));
      
      if (x >= tab_x && x < tab_x + width) {
        int rel_x = x - (tab_x + width);
        
        if (i >= 0 && y >= (TBWM_BAR_HEIGHT - 10) / 2 && y < ((TBWM_BAR_HEIGHT - 10) / 2) + 10) {
          if (rel_x >= -18 && rel_x < -8) {
            // TODO: close tab
            break;
          }
          
          rel_x += 18;
          
          if (tbwm_active < (sizeof(tbwm_bars) / sizeof(bar_t)) - 1) {
            if (rel_x >= -18 && rel_x < -8) {
              tab_t *move_tab = bar->tabs + i;
              
              tbwm_add_tab(tbwm_active + 1, move_tab->title, move_tab->data);
              tbwm_remove_tab(tbwm_active, move_tab->data);
              
              break;
            }
            
            rel_x += 18;
          }
          
          if (tbwm_active > 0) {
            if (rel_x >= -18 && rel_x < -8) {
              tab_t *move_tab = bar->tabs + i;
              
              tbwm_add_tab(tbwm_active - 1, move_tab->title, move_tab->data);
              tbwm_remove_tab(tbwm_active, move_tab->data);
              
              break;
            }
            
            rel_x += 18;
          }
        }
        
        i_select_tab(bar, i);
        break;
      }
      
      tab_x += width;
    }
    
    tbwm_redraw = 1;
  }
}

int main(void) {
  if (!i_init()) return 1;
  
  for (int i = 0; i < sizeof(tbwm_autostart) / sizeof(const char *); i++) {
    tbwm_spawn_text(tbwm_autostart[i]);
  }
  
  for (;;) {
    if (1 || tbwm_redraw) {
      for (int i = 0; i < sizeof(tbwm_bars) / sizeof(bar_t); i++) {
        tbwm_draw_bar(i);
      }
      
      tbwm_redraw = 0;
    }
    
    i_tick();
  }
  
  return 0;
}
