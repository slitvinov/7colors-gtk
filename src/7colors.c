#include <stdint.h>
#include <stdlib.h>
#include "blu.xpm"
#include "ciano.xpm"
#include "giallo.xpm"
#include "grigio.xpm"
#include "magenta.xpm"
#include "rosso.xpm"
#include "verde.xpm"
#include <gtk/gtk.h>

#define	USED(x)	if(x){}else{}
#define SIZE(x) (int)(sizeof (x)/sizeof *(x))
static const char *me = "7colors";
static void usg(void) {
  fprintf(stderr, "Usage: 7colors -[1|2] [c|h]\n");
  fprintf(stderr, " -1, -2   player 1 or 2\n");
  fprintf(stderr, " c, h     computer or human\n");
  fprintf(stderr, "Examples:\n");
  fprintf(stderr, "7colors -1 h -2 c\n");
  exit(1);
}
enum { ntab = 18, mtab = 18};
enum { HUMAN, COMPUTER };
static struct {
  int col;
  int segno;
} *tab[ntab];
static struct {
  int type;
  int col;
  int punti;
} pl[2];
static char **xpm[] = {
  rombo_grigio_xpm,
  rombo_rosso_xpm,
  rombo_verde_xpm,
  rombo_blu_xpm,
  rombo_ciano_xpm,
  rombo_magenta_xpm,
  rombo_giallo_xpm,
};

GtkWidget *canvas = NULL;
GdkPixmap *tavolagioco = NULL;
GdkPixmap *rombo[7];
GdkPixbuf *buf[7];
GdkBitmap *maschera = NULL;
GdkGC *miogc = NULL;
GtkWidget *lblpunti[2];
GtkWidget *bottonecol[7];
GtkWidget *statusbar;

int altezza_pixel, larghezza_pixel, altezza_rombo,
    larghezza_rombo;
int attivo;

static void gameover(void);
int colora(int x, int y, int old_col, int new_col);
int espandi(int x, int y, int col);
int guadagno(int x, int y, int old_col, int new_col);
int riempi2();
int riempi(int col);
int guadmax(int attivo);
void clear(void);
void disegna(int x, int y, int col);
void fill(int x, int y);
void mossa_computer(void);
void nuovo_gioco(void);
void scrivi_perc(int giocatore);
static gint expose_event(GtkWidget *widget, GdkEventExpose *event) {
  gdk_draw_pixmap(widget->window,
                  widget->style->fg_gc[GTK_WIDGET_STATE(widget)], tavolagioco,
                  event->area.x, event->area.y, event->area.x, event->area.y,
                  event->area.width, event->area.height);

  return FALSE;
}
void premuto_colore(GtkWidget *widget, gpointer colore) {
  USED(widget);
  gtk_widget_set_sensitive(bottonecol[pl[attivo].col], 1);
  gtk_widget_set_sensitive(bottonecol[(intptr_t)colore], 0);
  clear();
  pl[attivo].punti = colora(attivo * (ntab - 1),
                            ((attivo + 1) % 2) * (mtab - 1),
                            pl[attivo].col, (intptr_t)colore);
  fill(((attivo + 1) % 2) * (ntab - 1), attivo * (mtab - 1));
  pl[attivo].punti += riempi((intptr_t)colore);
  pl[attivo].col = (intptr_t)colore;
  scrivi_perc(attivo);
  if (pl[attivo].punti == ntab * mtab / 2 &&
      pl[(attivo + 1) % 2].punti == ntab * mtab / 2)
    gameover();
  else if (pl[attivo].punti > ntab * mtab / 2)
    gameover();
  else {
    attivo = (attivo + 1) % 2;
    if (pl[attivo].type == COMPUTER)
      mossa_computer();
  }
}
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
  for (i = 0; i < ntab; i++)
    if ((tab[i] = malloc(mtab * sizeof *tab[0])) == NULL) {
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
    buf[i] = gdk_pixbuf_new_from_xpm_data (xpm[i]);
    rombo[i] =
      gdk_pixmap_create_from_xpm_d
      (canvas->window, &maschera, &canvas->style->bg[GTK_STATE_NORMAL],
       xpm[i]);
  }
  miogc = gdk_gc_new(canvas->window);
  gdk_gc_set_clip_mask(miogc, maschera);
  gdk_window_get_size(maschera, &larghezza_rombo, &altezza_rombo);
  altezza_pixel = mtab * altezza_rombo / 2 + altezza_rombo / 2;
  larghezza_pixel = ntab * larghezza_rombo + larghezza_rombo / 2;
  gtk_drawing_area_size(GTK_DRAWING_AREA(canvas), larghezza_pixel,
                        altezza_pixel);
  tavolagioco =
      gdk_pixmap_new(canvas->window, larghezza_pixel, altezza_pixel, -1);
  gdk_draw_rectangle(tavolagioco, window->style->black_gc, TRUE, 0, 0,
                     larghezza_pixel, altezza_pixel);
  g_signal_connect(canvas, "expose_event", G_CALLBACK(expose_event), NULL);
  separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(container1), separator, FALSE, TRUE, 0);
  gtk_widget_show(separator);
  container2 = gtk_hbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(container1), container2, FALSE, FALSE, 0);
  gtk_widget_show(container2);
  for (i = 0; i < 7; i++) {
    bottonecol[i] = gtk_button_new();
    container3 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(container3);
    pixmapwid = gtk_pixmap_new(rombo[i], maschera);
    gtk_widget_show(pixmapwid);
    gtk_box_pack_start(GTK_BOX(container3), pixmapwid, FALSE, FALSE, 0);
    gtk_widget_show(container3);
    gtk_container_add(GTK_CONTAINER(bottonecol[i]), container3);
    gtk_box_pack_start(GTK_BOX(container2), bottonecol[i], TRUE, TRUE, 10);
    g_signal_connect(bottonecol[i], "clicked", G_CALLBACK(premuto_colore), (gpointer)i);
    gtk_widget_set_sensitive(bottonecol[i], 0);
    gtk_widget_show(bottonecol[i]);
  }
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

void nuovo_gioco(void) {
  int x, y, i;
  int col;
  for (y = 0; y < mtab; y++) {
    for (x = 0; x < ntab; x++) {
      col = 7.0 * rand() / (RAND_MAX + 1.0);
      tab[x][y].col = col;
      disegna(x, y, col);
      tab[x][y].segno = 0;
    }
  }
  pl[0].col = tab[0][mtab - 1].col;
  pl[1].col = tab[ntab - 1][0].col;
  if (pl[1].col == pl[0].col) {
    pl[1].col = (pl[1].col + 1) % 7 + 1;
    tab[ntab - 1][0].col = pl[1].col;
    disegna(ntab - 1, 0, pl[1].col);
  }
  pl[0].punti = espandi(0, mtab - 1, pl[0].col);
  scrivi_perc(0);
  pl[1].punti = espandi(ntab - 1, 0, pl[1].col);
  scrivi_perc(1);
  attivo = 0;
  for (i = 0; i < 7; i++)
    gtk_widget_set_sensitive(bottonecol[i], 1);
  gtk_widget_set_sensitive(bottonecol[pl[0].col], 0);
  gtk_widget_set_sensitive(bottonecol[pl[1].col], 0);
  if (pl[attivo].type == COMPUTER)
    mossa_computer();
}

void disegna(int x, int y, int col) {
  GdkRectangle update_rect;
  cairo_t *cr;
  x = x * larghezza_rombo + ((y + 1) % 2) * larghezza_rombo / 2;
  y = y * altezza_rombo / 2;
  update_rect.x = x;
  update_rect.y = y;
  update_rect.width = larghezza_rombo;
  update_rect.height = altezza_rombo;
  cr = gdk_cairo_create(tavolagioco);
  gdk_cairo_set_source_pixbuf(cr, buf[col], x, y);
  gdk_cairo_rectangle(cr, &update_rect);
  cairo_paint(cr);
  cairo_destroy(cr);
  gtk_widget_draw(canvas, &update_rect);
}

int colora(int x, int y, int old_col, int new_col) {
  int k = 0;
  if (x < ntab && x >= 0 && y < mtab && y >= 0) {
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

void clear(void) {
  int i, j;
  for (i = 0; i < ntab; i++)
    for (j = 0; j < mtab; j++)
      tab[i][j].segno = 0;
}

int espandi(int x, int y, int col) {
  int k = 0;

  if (x < ntab && x >= 0 && y < mtab && y >= 0) {
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
  if (x < ntab && x >= 0 && y < mtab && y >= 0) {
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

int riempi(int col) {
  int x, y, k = 0;
  for (x = 0; x < ntab; x++) {
    for (y = 0; y < mtab; y++) {
      if (tab[x][y].segno == 0) {
        k++;
        tab[x][y].col = col;
        disegna(x, y, col);
      }
    }
  }
  return k;
}

int guadmax(int attivo) {
  int imax, i;
  int c_max, c;

  c_max = 0;
  imax = 0;
  for (i = 0; i < 7; i++) {
    if (i != pl[(attivo + 1) % 2].col && i != pl[attivo].col) {
      clear();
      c = guadagno(attivo * (ntab - 1),
                   ((attivo + 1) % 2) * (mtab - 1), pl[attivo].col, i);
      fill(((attivo + 1) % 2) * (ntab - 1),
           attivo * (mtab - 1));
      c += riempi2();

      if (c >= c_max) {
        c_max = c;
        imax = i;
      }
    }
  }
  return imax;
}

int guadagno(int x, int y, int old_col, int new_col) {
  int k = 0;

  if (x < ntab && x >= 0 && y < mtab && y >= 0) {
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

int riempi2() {
  int x, y, k = 0;

  for (x = 0; x < ntab; x++) {
    for (y = 0; y < mtab; y++) {
      if (tab[x][y].segno == 0) {
        k++;
        tab[x][y].segno = 1;
      }
    }
  }
  return k;
}

void scrivi_perc(int giocatore) {
  char perc[10];

  sprintf(perc, "%03.1f%%",
          pl[giocatore].punti * 100.0 / (ntab * mtab));
  gtk_label_set_text((GtkLabel *)lblpunti[giocatore], perc);
}

void mossa_computer(void) {
  int colore;
  colore = guadmax(attivo);
  gtk_widget_set_sensitive(bottonecol[pl[attivo].col], 1);
  gtk_widget_set_sensitive(bottonecol[(int)colore], 0);
  clear();
  pl[attivo].punti = colora(attivo * (ntab - 1),
                            ((attivo + 1) % 2) * (mtab - 1),
                            pl[attivo].col, (int)colore);
  fill(((attivo + 1) % 2) * (ntab - 1), attivo * (mtab - 1));
  pl[attivo].punti += riempi((int)colore);
  pl[attivo].col = (int)colore;
  scrivi_perc(attivo);

  if (pl[attivo].punti == ntab * mtab / 2 &&
      pl[(attivo + 1) % 2].punti == ntab * mtab / 2)
    gameover();
  else if (pl[attivo].punti > ntab * mtab / 2)
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

