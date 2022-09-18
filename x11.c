#ifdef I_X11

#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <string.h>
#include <tbwm.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static Display *display;
static int screen;

static Window root, bar;

static GC graphic_ctx;
static Drawable drawable;

static int has_updated;

int font_ascent;
int font_descent;
int font_width;

XFontStruct *regular_font, *bold_font;

static Cursor cursor;

void i_draw_rect(int x, int y, int width, int height, color_t color) {
  XRectangle rect = {x, y, width, height};
  
  XSetForeground(display, graphic_ctx, color);
  XFillRectangles(display, drawable, graphic_ctx, &rect, 1);
  
  has_updated = 1;
}

void i_draw_text(const char *text, int x, int y, int max_length, int center, int bold, color_t color) {
  XSetForeground(display, graphic_ctx, color);
  
  if (bold) {
    XSetFont(display, graphic_ctx, bold_font->fid);
  } else {
    XSetFont(display, graphic_ctx, regular_font->fid);
  }
  
  XDrawString(display, drawable, graphic_ctx, x, y + font_ascent, text, strlen(text));
  
  has_updated = 1;
}

void i_draw_line(int x1, int y1, int x2, int y2, color_t color) {
  XSetForeground(display, graphic_ctx, color);
  
  XDrawLine(display, drawable, graphic_ctx, x1, y1, x2, y2);
  XDrawPoint(display, drawable, graphic_ctx, x2, y2);
  
  has_updated = 1;
}

int i_get_width(void) {
  return DisplayWidth(display, screen);
}

int i_get_height(void) {
  return DisplayHeight(display, screen);
}

int i_get_text_width(void) {
  return font_width;
}

int i_get_text_height(void) {
  return font_ascent + font_descent;
}

void i_select_tab(bar_t *bar, int index) {
  if (index < 0) {
    for (int i = 0; i < bar->count; i++) {
      XUnmapWindow(display, bar->tabs[i].data);
    }
  } else {
    XMoveResizeWindow(display, bar->tabs[index].data, bar->x, TBWM_BAR_HEIGHT, bar->width, bar->screen_height - TBWM_BAR_HEIGHT);
    XMapRaised(display, bar->tabs[index].data);
  }
  
  XSync(display, False);
  bar->active = index;
}

int i_init(void) {
  display = XOpenDisplay(NULL);
  if (!display) return 0;
  
  screen = DefaultScreen(display);
  root = RootWindow(display, screen);
  
  drawable = XCreatePixmap(display, root, i_get_width(), TBWM_BAR_HEIGHT, DefaultDepth(display, screen));
  graphic_ctx = XCreateGC(display, root, 0, NULL);
  
  XSetLineAttributes(display, graphic_ctx, 1, LineSolid, CapButt, JoinMiter);
  
  XSetWindowAttributes wa;
  
  wa.override_redirect = True;
  wa.background_pixmap = ParentRelative;
  wa.event_mask = ButtonPressMask | ExposureMask;
  
  bar = XCreateWindow(display, root, 0, 0, i_get_width(), TBWM_BAR_HEIGHT, 0, DefaultDepth(display, screen),
                          CopyFromParent, DefaultVisual(display, screen),
                          CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
  
  XMapRaised(display, bar);
  XSync(display, False);
  
  const char *regular_font_name = TBWM_REGULAR_FONT "--" TBWM_FONT_SIZE "-*-*-*-c-*-iso10646-1";
  const char *bold_font_name = TBWM_BOLD_FONT "--" TBWM_FONT_SIZE "-*-*-*-c-*-iso10646-1";
  
  regular_font = XLoadQueryFont(display, regular_font_name);
  if (!regular_font) return 0;
  
  bold_font = XLoadQueryFont(display, bold_font_name);
  
  if (!bold_font) {
    bold_font = XLoadQueryFont(display, regular_font_name);
  }
  
  font_width = MAX(regular_font->max_bounds.width, bold_font->max_bounds.width);
  font_ascent = MAX(regular_font->ascent, bold_font->ascent);
  font_descent = MAX(regular_font->descent, bold_font->descent);
  
  cursor = XCreateFontCursor(display, XC_left_ptr);
  
  wa.cursor = cursor;
  wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ButtonPressMask
                |EnterWindowMask|LeaveWindowMask|StructureNotifyMask
                |PropertyChangeMask | ButtonPressMask | ButtonReleaseMask;
  
  XChangeWindowAttributes(display, root, CWEventMask | CWCursor, &wa);
  XSelectInput(display, root, wa.event_mask);
  
  has_updated = 0;
  
  return 1;
}

static void i_update_title(i_window_t window) {
  tab_t *tab = tbwm_find_tab(-1, window);
  
  if (tab) {
    char *buffer;
    XFetchName(display, window, &buffer);
    
    if (buffer) {
      tbwm_set_tab_title(tab, buffer);
      XFree(buffer);
    } else {
      XGetIconName(display, window, &buffer);
      
      if (buffer) {
        tbwm_set_tab_title(tab, buffer);
        XFree(buffer);
      }
    }
  }
}

void i_tick(void) {
  XEvent event;
  XNextEvent(display, &event);
  
  if (event.type == ConfigureRequest) {
    if (!tbwm_find_tab(-1, event.xconfigure.window)) {
      tbwm_add_tab(tbwm_active, "(no title)", event.xconfigure.window);
      i_update_title(event.xconfigure.window);
    }
  } else if (event.type == DestroyNotify) {
    tbwm_remove_tab(-1, event.xdestroywindow.window);
  } else if (event.type == ButtonPress) {
    tbwm_on_click(event.xbutton.window, event.xmotion.x_root, event.xmotion.y_root);
  } else if (event.type == PropertyNotify) {
    i_update_title(event.xproperty.window);
  }
  
  if (has_updated) {
    XCopyArea(display, drawable, bar, graphic_ctx, 0, 0, i_get_width(), TBWM_BAR_HEIGHT, 0, 0);
    XSync(display, False);
  }
  
  has_updated = 0;
}

#endif
