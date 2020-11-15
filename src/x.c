#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <xcb/xcb.h>
#define	USED(x)	if(x){}else{}
#define SIZE(x) (int)(sizeof(x) / sizeof *(x))
static const char *me = "7colors";
static void usg(void) {
  fprintf(stderr, "Usage: 7colors -[1|2] [c|h]\n");
  fprintf(stderr, " -1, -2   player 1 or 2\n");
  fprintf(stderr, " c, h     computer or human\n");
  fprintf(stderr, "Examples:\n");
  fprintf(stderr, "7colors -1 h -2 c\n");
  exit(1);
}
enum { HUMAN, COMPUTER };
static struct { int type; } pl[2];
int main(int argc, char *argv[]) {
  xcb_connection_t *c;
  const xcb_setup_t *s;
  xcb_screen_t *screen;
  xcb_screen_iterator_t iter;
  xcb_window_t win;
  int screen_count;
  int screen_default;

  USED(argc);
  pl[0].type = HUMAN;
  pl[1].type = COMPUTER;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      usg();
      break;
    case '1':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "%s: -2 needs an argument\n", me);
        exit(2);
      }
      switch (argv[0][0]) {
      case 'c':
        pl[0].type = COMPUTER;
        break;
      case 'h':
        pl[0].type = HUMAN;
        break;
      default:
        fprintf(stderr, "%s: unknown type '%c'\n", me, argv[0][0]);
        exit(2);
      }
      break;
    case '2':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "%s: -2 needs an argument\n", me);
        exit(2);
      }
      switch (argv[0][0]) {
      case 'c':
        pl[1].type = COMPUTER;
        break;
      case 'h':
        pl[1].type = HUMAN;
        break;
      default:
        fprintf(stderr, "%s: unknown type '%c'\n", me, argv[0][0]);
        exit(2);
      }
      break;
    default:
      fprintf(stderr, "%s: unknown option '%s'\n", me, argv[0]);
      exit(2);
      break;
    }

  c = xcb_connect(NULL, &screen_default);
  if (xcb_connection_has_error(c) != 0) {
    fprintf(stderr, "%s: can't connect to X display\n", me);
    exit(2);
  }
  s = xcb_get_setup(c);
  screen_count = xcb_setup_roots_length(s);
  iter = xcb_setup_roots_iterator(s);
  for (; iter.rem; --screen_default, xcb_screen_next(&iter))
    if (screen_default == 0) {
      screen = iter.data;
      break;
    }
  win = xcb_generate_id(c);
  /* Create the window */
  xcb_create_window(c,                    /* Connection          */
                    XCB_COPY_FROM_PARENT, /* depth (same as root)*/
                    win,                  /* window Id           */
                    screen->root,         /* parent window       */
                    0, 0,                 /* x, y                */
                    screen->width_in_pixels / 2, screen->height_in_pixels / 3,
                    10,                            /* border_width        */
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                    screen->root_visual,           /* visual              */
                    0, NULL);                      /* masks, not used yet */
  /* Map the window on the screen */
  xcb_map_window(c, win);
  xcb_flush(c);
  fprintf(stderr, "screen_count: %d\n", screen_count);
  fprintf(stderr, "screen_default: %d\n", screen_default);
  printf("width: %d\n", screen->width_in_pixels);
  printf("height: %d\n", screen->height_in_pixels);
  getc(stdin);
  xcb_disconnect(c);
  srand(time(NULL));
}
