#include "blu.xpm"
#include "ciano.xpm"
#include "giallo.xpm"
#include "grigio.xpm"
#include "magenta.xpm"
#include "rosso.xpm"
#include "verde.xpm"
#include <gtk/gtk.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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
enum { mtab = 18, ntab = 18, mrombo = 32, nrombo = 32, npixel = ntab * nrombo / 2 + nrombo / 2, mpixel = mtab * mrombo + mrombo / 2 };
enum { HUMAN, COMPUTER };
static struct {
  int col;
  int segno;
} * tab[mtab];
static struct {
  int type;
  int col;
  int punti;
} pl[2];
static char **xpm[] = {
    rombo_grigio_xpm, rombo_rosso_xpm,   rombo_verde_xpm,  rombo_blu_xpm,
    rombo_ciano_xpm,  rombo_magenta_xpm, rombo_giallo_xpm,
};

GtkWidget *canvas;
GdkPixmap *board;
GdkPixmap *rombo[7];
GdkPixbuf *buf[7];
GdkBitmap *mask = NULL;
GtkWidget *lblpunti[2];
GtkWidget *bottonecol[7];
GtkWidget *statusbar;

int attivo;
static void gameover(void);
static int colora(int, int, int, int);
static int espandi(int, int, int);
static int guadagno(int, int, int, int);
static int riempi2(void);
static int riempi(int);
static int guadmax(int);
static void clear(void);
static void disegna(int, int, int);
static void fill(int, int);
static void mossa_computer(void);
static void nuovo_gioco(void);
static void scrivi_perc(int);
static int expose_event(GtkWidget*, GdkEventExpose *);
static void premuto_colore(GtkWidget *, gpointer);
int main(int argc, char *argv[]) {
  GtkWidget *window;
  GtkWidget *container1;
  GtkWidget *container2;
  GtkWidget *container3;
  GtkWidget *bottone;
  GtkWidget *separator;
  GtkWidget *pixmapwid;
  int i;
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
  for (i = 0; i < mtab; i++)
    if ((tab[i] = malloc(ntab * sizeof *tab[0])) == NULL) {
      fprintf(stderr, "%s: fail to allocate memory\n", me);
      return 1;
    }
  srand(time(NULL));
  if (gtk_init_check(&argc, &argv) == 0) {
    fprintf(stderr, "%s: fail to initialize GUI\n", me);
    return 1;
  }
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  gtk_window_set_title(GTK_WINDOW(window), "7Colors");
  gtk_window_set_resizable(GTK_WINDOW(window), 0);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  container1 = gtk_vbox_new(FALSE, 10);
  gtk_container_add(GTK_CONTAINER(window), container1);
  container2 = gtk_hbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(container1), container2, FALSE, FALSE, 0);
  lblpunti[0] = gtk_label_new("0\%");
  gtk_box_pack_start(GTK_BOX(container2), lblpunti[0], TRUE, TRUE, 0);
  lblpunti[1] = gtk_label_new("0\%");
  gtk_box_pack_start(GTK_BOX(container2), lblpunti[1], TRUE, TRUE, 0);
  canvas = gtk_drawing_area_new();
  gtk_box_pack_start(GTK_BOX(container1), canvas, FALSE, FALSE, 0);
  gtk_widget_show_all(window);
  for (i = 0; i < SIZE(xpm); i++) {
    buf[i] = gdk_pixbuf_new_from_xpm_data((const char **)xpm[i]);
    rombo[i] = gdk_pixmap_create_from_xpm_d(
        canvas->window, &mask, &canvas->style->bg[GTK_STATE_NORMAL], xpm[i]);
  }
  gtk_widget_set_size_request(canvas, mpixel, npixel);
  board =
      gdk_pixmap_new(canvas->window, mpixel, npixel, -1);
  g_signal_connect(canvas, "expose_event", G_CALLBACK(expose_event), NULL);
  separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(container1), separator, FALSE, TRUE, 0);
  gtk_widget_show(separator);
  container2 = gtk_hbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(container1), container2, FALSE, FALSE, 0);
  for (i = 0; i < 7; i++) {
    bottonecol[i] = gtk_button_new();
    container3 = gtk_vbox_new(FALSE, 0);
    pixmapwid = gtk_pixmap_new(rombo[i], mask);
    gtk_box_pack_start(GTK_BOX(container3), pixmapwid, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(bottonecol[i]), container3);
    gtk_box_pack_start(GTK_BOX(container2), bottonecol[i], TRUE, TRUE, 10);
    g_signal_connect(bottonecol[i], "clicked", G_CALLBACK(premuto_colore),
                     (void *)(intptr_t)i);
    gtk_widget_set_sensitive(bottonecol[i], 0);
  }
  gtk_widget_show_all(container2);
  separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(container1), separator, FALSE, TRUE, 0);
  gtk_widget_show(separator);
  container2 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(container1), container2, FALSE, FALSE, 0);
  gtk_widget_show(container2);
  bottone = gtk_button_new_with_label("New");
  gtk_box_pack_start(GTK_BOX(container2), bottone, TRUE, FALSE, 10);
  g_signal_connect(bottone, "clicked", GTK_SIGNAL_FUNC(nuovo_gioco), NULL);
  gtk_widget_show(bottone);
  bottone = gtk_button_new_with_label("Exit");
  gtk_box_pack_start(GTK_BOX(container2), bottone, TRUE, FALSE, 10);
  gtk_signal_connect_object(GTK_OBJECT(bottone), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy),
                            GTK_OBJECT(window));
  gtk_widget_show(bottone);
  gtk_main();
}

static void nuovo_gioco(void) {
  int x, y, i;
  int col;
  for (y = 0; y < ntab; y++) {
    for (x = 0; x < mtab; x++) {
      col = 7.0 * rand() / (RAND_MAX + 1.0);
      tab[x][y].col = col;
      disegna(x, y, col);
      tab[x][y].segno = 0;
    }
  }
  pl[0].col = tab[0][ntab - 1].col;
  pl[1].col = tab[mtab - 1][0].col;
  if (pl[1].col == pl[0].col) {
    pl[1].col = (pl[1].col + 1) % 7 + 1;
    tab[mtab - 1][0].col = pl[1].col;
    disegna(mtab - 1, 0, pl[1].col);
  }
  pl[0].punti = espandi(0, ntab - 1, pl[0].col);
  scrivi_perc(0);
  pl[1].punti = espandi(mtab - 1, 0, pl[1].col);
  scrivi_perc(1);
  attivo = 0;
  for (i = 0; i < 7; i++)
    gtk_widget_set_sensitive(bottonecol[i], 1);
  gtk_widget_set_sensitive(bottonecol[pl[0].col], 0);
  gtk_widget_set_sensitive(bottonecol[pl[1].col], 0);
  if (pl[attivo].type == COMPUTER)
    mossa_computer();
}

static void disegna(int x, int y, int col) {
  GdkRectangle update_rect;
  cairo_t *cr;
  x = x * mrombo + ((y + 1) % 2) * mrombo / 2;
  y = y * nrombo / 2;
  update_rect.x = x;
  update_rect.y = y;
  update_rect.width = mrombo;
  update_rect.height = nrombo;
  cr = gdk_cairo_create(GDK_DRAWABLE(board));
  gdk_cairo_set_source_pixbuf(cr, buf[col], x, y);
  gdk_cairo_rectangle(cr, &update_rect);
  cairo_paint(cr);
  cairo_destroy(cr);
  gtk_widget_draw(canvas, &update_rect);
}

static int colora(int x, int y, int old_col, int new_col) {
  int k = 0;
  if (x < mtab && x >= 0 && y < ntab && y >= 0) {
    if (tab[x][y].col == old_col) {
      tab[x][y].col = new_col;
      disegna(x, y, new_col);
      tab[x][y].segno = 1;
      if (y % 2 == 0)
        k = colora(x, y - 1, old_col, new_col) +
            colora(x, y + 1, old_col, new_col) +
            colora(x + 1, y - 1, old_col, new_col) +
            colora(x + 1, y + 1, old_col, new_col) + 1;
      else
        k = colora(x, y - 1, old_col, new_col) +
            colora(x, y + 1, old_col, new_col) +
            colora(x - 1, y - 1, old_col, new_col) +
            colora(x - 1, y + 1, old_col, new_col) + 1;
    } else {
      if (tab[x][y].col == new_col && tab[x][y].segno == 0) {
        tab[x][y].segno = 1;
        if (y % 2 == 0)
          k = espandi(x, y - 1, new_col) + espandi(x, y + 1, new_col) +
              espandi(x + 1, y - 1, new_col) + espandi(x + 1, y + 1, new_col) +
              1;
        else
          k = espandi(x, y - 1, new_col) + espandi(x, y + 1, new_col) +
              espandi(x - 1, y - 1, new_col) + espandi(x - 1, y + 1, new_col) +
              1;
      }
    }
  }
  return k;
}

static void clear(void) {
  int i, j;
  for (i = 0; i < mtab; i++)
    for (j = 0; j < ntab; j++)
      tab[i][j].segno = 0;
}

static int espandi(int x, int y, int col) {
  int k = 0;

  if (x < mtab && x >= 0 && y < ntab && y >= 0) {
    if (tab[x][y].col == col && tab[x][y].segno == 0) {
      tab[x][y].segno = 1;
      if (y % 2 == 0)
        k = espandi(x, y - 1, col) + espandi(x, y + 1, col) +
            espandi(x + 1, y - 1, col) + espandi(x + 1, y + 1, col) + 1;
      else
        k = espandi(x, y - 1, col) + espandi(x, y + 1, col) +
            espandi(x - 1, y - 1, col) + espandi(x - 1, y + 1, col) + 1;
    }
  }
  return k;
}

void fill(int x, int y) {
  if (x < mtab && x >= 0 && y < ntab && y >= 0) {
    if (tab[x][y].segno == 0) {
      tab[x][y].segno = 1;
      if (y % 2 == 0) {
        fill(x, y - 1);
        fill(x, y + 1);
        fill(x + 1, y - 1);
        fill(x + 1, y + 1);
      } else {
        fill(x, y - 1);
        fill(x, y + 1);
        fill(x - 1, y - 1);
        fill(x - 1, y + 1);
      }
    }
  }
}

static int riempi(int col) {
  int x, y, k = 0;
  for (x = 0; x < mtab; x++) {
    for (y = 0; y < ntab; y++) {
      if (tab[x][y].segno == 0) {
        k++;
        tab[x][y].col = col;
        disegna(x, y, col);
      }
    }
  }
  return k;
}

static int guadmax(int attivo) {
  int imax, i;
  int c_max, c;

  c_max = 0;
  imax = 0;
  for (i = 0; i < 7; i++) {
    if (i != pl[(attivo + 1) % 2].col && i != pl[attivo].col) {
      clear();
      c = guadagno(attivo * (mtab - 1), ((attivo + 1) % 2) * (ntab - 1),
                   pl[attivo].col, i);
      fill(((attivo + 1) % 2) * (mtab - 1), attivo * (ntab - 1));
      c += riempi2();

      if (c >= c_max) {
        c_max = c;
        imax = i;
      }
    }
  }
  return imax;
}

static int guadagno(int x, int y, int old_col, int new_col) {
  int k = 0;

  if (x < mtab && x >= 0 && y < ntab && y >= 0) {
    if (tab[x][y].col == old_col && tab[x][y].segno == 0) {
      tab[x][y].segno = 1;
      if (y % 2 == 0)
        k = guadagno(x, y - 1, old_col, new_col) +
            guadagno(x, y + 1, old_col, new_col) +
            guadagno(x + 1, y - 1, old_col, new_col) +
            guadagno(x + 1, y + 1, old_col, new_col) + 1;
      else
        k = guadagno(x, y - 1, old_col, new_col) +
            guadagno(x, y + 1, old_col, new_col) +
            guadagno(x - 1, y - 1, old_col, new_col) +
            guadagno(x - 1, y + 1, old_col, new_col) + 1;
    } else {
      if (tab[x][y].col == new_col && tab[x][y].segno == 0) {
        tab[x][y].segno = 1;
        if (y % 2 == 0)
          k = espandi(x, y - 1, new_col) + espandi(x, y + 1, new_col) +
              espandi(x + 1, y - 1, new_col) + espandi(x + 1, y + 1, new_col) +
              1;
        else
          k = espandi(x, y - 1, new_col) + espandi(x, y + 1, new_col) +
              espandi(x - 1, y - 1, new_col) + espandi(x - 1, y + 1, new_col) +
              1;
      }
    }
  }
  return k;
}

static int riempi2() {
  int x, y, k = 0;

  for (x = 0; x < mtab; x++) {
    for (y = 0; y < ntab; y++) {
      if (tab[x][y].segno == 0) {
        k++;
        tab[x][y].segno = 1;
      }
    }
  }
  return k;
}

static void scrivi_perc(int giocatore) {
  char perc[10];

  sprintf(perc, "%03.1f%%", pl[giocatore].punti * 100.0 / (mtab * ntab));
  gtk_label_set_text((GtkLabel *)lblpunti[giocatore], perc);
}

static void mossa_computer(void) {
  int colore;
  colore = guadmax(attivo);
  gtk_widget_set_sensitive(bottonecol[pl[attivo].col], 1);
  gtk_widget_set_sensitive(bottonecol[(int)colore], 0);
  clear();
  pl[attivo].punti =
      colora(attivo * (mtab - 1), ((attivo + 1) % 2) * (ntab - 1),
             pl[attivo].col, (int)colore);
  fill(((attivo + 1) % 2) * (mtab - 1), attivo * (ntab - 1));
  pl[attivo].punti += riempi((int)colore);
  pl[attivo].col = (int)colore;
  scrivi_perc(attivo);

  if (pl[attivo].punti == mtab * ntab / 2 &&
      pl[(attivo + 1) % 2].punti == mtab * ntab / 2)
    gameover();
  else if (pl[attivo].punti > mtab * ntab / 2)
    gameover();
  else {
    attivo = (attivo + 1) % 2;
    if (pl[attivo].type == COMPUTER)
      mossa_computer();
  }
}

static void gameover(void) {
  int i;
  for (i = 0; i < 7; i++)
    gtk_widget_set_sensitive(bottonecol[i], 0);
}

static int expose_event(GtkWidget *widget, GdkEventExpose *event) {
  gdk_draw_pixmap(widget->window,
                  widget->style->fg_gc[GTK_WIDGET_STATE(widget)], board,
                  event->area.x, event->area.y, event->area.x, event->area.y,
                  event->area.width, event->area.height);

  return 0;
}

static void premuto_colore(GtkWidget *widget, gpointer colore) {
  USED(widget);
  gtk_widget_set_sensitive(bottonecol[pl[attivo].col], 1);
  gtk_widget_set_sensitive(bottonecol[(intptr_t)colore], 0);
  clear();
  pl[attivo].punti =
      colora(attivo * (mtab - 1), ((attivo + 1) % 2) * (ntab - 1),
             pl[attivo].col, (intptr_t)colore);
  fill(((attivo + 1) % 2) * (mtab - 1), attivo * (ntab - 1));
  pl[attivo].punti += riempi((intptr_t)colore);
  pl[attivo].col = (intptr_t)colore;
  scrivi_perc(attivo);
  if (pl[attivo].punti == mtab * ntab / 2 &&
      pl[(attivo + 1) % 2].punti == mtab * ntab / 2)
    gameover();
  else if (pl[attivo].punti > mtab * ntab / 2)
    gameover();
  else {
    attivo = (attivo + 1) % 2;
    if (pl[attivo].type == COMPUTER)
      mossa_computer();
  }
}
