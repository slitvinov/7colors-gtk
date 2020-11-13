#include "blu.xpm"
#include "ciano.xpm"
#include "giallo.xpm"
#include "grigio.xpm"
#include "magenta.xpm"
#include "rosso.xpm"
#include "verde.xpm"
#include <gtk/gtk.h>
#include <stdlib.h>

const char *me = "7colors";

enum { HUMAN, COMPUTER };
struct tgiocatore {
  int type;
  short int col;
  int punti;
};

struct ttab {
  short int col;
  short int segno;
};

static char *nomecolore[] = {"Gray", "Red",     "Green", "Blue",
                             "Cyan", "Magenta", "Yellow"};

GtkWidget *areadisegno = NULL;
GdkPixmap *tavolagioco = NULL;
GdkPixmap *rombo[7];
GdkBitmap *maschera = NULL;
GdkGC *miogc = NULL;
GtkWidget *lblpunti[2];
GtkWidget *bottonecol[7];
GtkWidget *statusbar;

struct ttab **tab;
struct tgiocatore pl[2];

int altezza_tab, larghezza_tab, altezza_pixel, larghezza_pixel, altezza_rombo,
    larghezza_rombo;
short int attivo;

int colora(int x, int y, short int old_col, short int new_col);
int espandi(int x, int y, short int col);
int guadagno(int x, int y, short int old_col, short int new_col);
int leggi_config(int argc, char *[]);
int riempi2();
int riempi(short int col);
short int guadmax(short int attivo);
void clear(void);
void cmderror(void);
void disegna(int x, int y, short int col);
void fill(int x, int y);
void gameover(int giocatore);
void mossa_computer(void);
void nuovo_gioco(void);
void scrivi_perc(short int giocatore);

gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
  return (FALSE);
}

void destroy(GtkWidget *widget, gpointer data) { gtk_main_quit(); }
static gint expose_event(GtkWidget *widget, GdkEventExpose *event) {
  gdk_draw_pixmap(widget->window,
                  widget->style->fg_gc[GTK_WIDGET_STATE(widget)], tavolagioco,
                  event->area.x, event->area.y, event->area.x, event->area.y,
                  event->area.width, event->area.height);

  return FALSE;
}
void nuovo_click_event(GtkWidget *widget, gpointer data) {
  nuovo_gioco();
}

void premuto_colore(GtkWidget *widget, gpointer colore) {
  gtk_widget_set_sensitive(bottonecol[pl[attivo].col], 1);
  gtk_widget_set_sensitive(bottonecol[(int)colore], 0);
  clear();
  pl[attivo].punti = colora(attivo * (larghezza_tab - 1),
                            ((attivo + 1) % 2) * (altezza_tab - 1),
                            pl[attivo].col, (int)colore);
  fill(((attivo + 1) % 2) * (larghezza_tab - 1), attivo * (altezza_tab - 1));
  pl[attivo].punti += riempi((int)colore);
  pl[attivo].col = (int)colore;
  scrivi_perc(attivo);
  if (pl[attivo].punti == larghezza_tab * altezza_tab / 2 &&
      pl[(attivo + 1) % 2].punti == larghezza_tab * altezza_tab / 2)
    gameover(2);
  else if (pl[attivo].punti > larghezza_tab * altezza_tab / 2)
    gameover(attivo);
  else {
    attivo = (attivo + 1) % 2;
    if (pl[attivo].type == COMPUTER)
      mossa_computer();
  }
}
int main(int argc, char *argv[]) {
  GtkWidget *window;
  GtkWidget *contenitore1;
  GtkWidget *contenitore2;
  GtkWidget *contenitore3;
  GtkWidget *bottone;
  GtkWidget *separator;
  GtkWidget *pixmapwid;
  GtkWidget *label;
  int i;
  gtk_init(&argc, &argv);
  leggi_config(argc, argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect(GTK_OBJECT(window), "delete_event",
                     GTK_SIGNAL_FUNC(delete_event), NULL);
  gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(destroy),
                     NULL);
  gtk_window_set_title(GTK_WINDOW(window), "7Colors");
  gtk_window_set_policy(GTK_WINDOW(window), 0, 0, 0); // no resize
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_widget_show(window);
  contenitore1 = gtk_vbox_new(FALSE, 10);
  gtk_container_add(GTK_CONTAINER(window), contenitore1);
  gtk_widget_show(contenitore1);
  contenitore2 = gtk_hbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(contenitore1), contenitore2, FALSE, FALSE, 0);
  gtk_widget_show(contenitore2);
  lblpunti[0] = gtk_label_new("0\%");
  gtk_box_pack_start(GTK_BOX(contenitore2), lblpunti[0], TRUE, TRUE, 0);
  gtk_widget_show(lblpunti[0]);
  lblpunti[1] = gtk_label_new("0\%");
  gtk_box_pack_start(GTK_BOX(contenitore2), lblpunti[1], TRUE, TRUE, 0);
  gtk_widget_show(lblpunti[1]);
  areadisegno = gtk_drawing_area_new();
  gtk_box_pack_start(GTK_BOX(contenitore1), areadisegno, FALSE, FALSE, 0);
  gtk_widget_show(areadisegno);
  rombo[0] = gdk_pixmap_create_from_xpm_d(
      areadisegno->window, &maschera, &areadisegno->style->bg[GTK_STATE_NORMAL],
      (gchar **)rombo_grigio_xpm);
  rombo[1] = gdk_pixmap_create_from_xpm_d(
      areadisegno->window, &maschera, &areadisegno->style->bg[GTK_STATE_NORMAL],
      (gchar **)rombo_rosso_xpm);
  rombo[2] = gdk_pixmap_create_from_xpm_d(
      areadisegno->window, &maschera, &areadisegno->style->bg[GTK_STATE_NORMAL],
      (gchar **)rombo_verde_xpm);
  rombo[3] = gdk_pixmap_create_from_xpm_d(
      areadisegno->window, &maschera, &areadisegno->style->bg[GTK_STATE_NORMAL],
      (gchar **)rombo_blu_xpm);
  rombo[4] = gdk_pixmap_create_from_xpm_d(
      areadisegno->window, &maschera, &areadisegno->style->bg[GTK_STATE_NORMAL],
      (gchar **)rombo_ciano_xpm);
  rombo[5] = gdk_pixmap_create_from_xpm_d(
      areadisegno->window, &maschera, &areadisegno->style->bg[GTK_STATE_NORMAL],
      (gchar **)rombo_magenta_xpm);
  rombo[6] = gdk_pixmap_create_from_xpm_d(
      areadisegno->window, &maschera, &areadisegno->style->bg[GTK_STATE_NORMAL],
      (gchar **)rombo_giallo_xpm);
  miogc = gdk_gc_new(areadisegno->window);
  gdk_gc_set_clip_mask(miogc, maschera);
  gdk_window_get_size(maschera, &larghezza_rombo, &altezza_rombo);
  altezza_pixel = altezza_tab * altezza_rombo / 2 + altezza_rombo / 2;
  larghezza_pixel = larghezza_tab * larghezza_rombo + larghezza_rombo / 2;
  gtk_drawing_area_size(GTK_DRAWING_AREA(areadisegno), larghezza_pixel,
                        altezza_pixel);
  tavolagioco =
      gdk_pixmap_new(areadisegno->window, larghezza_pixel, altezza_pixel, -1);
  gdk_draw_rectangle(tavolagioco, window->style->black_gc, TRUE, 0, 0,
                     larghezza_pixel, altezza_pixel);
  gtk_signal_connect(GTK_OBJECT(areadisegno), "expose_event",
                     (GtkSignalFunc)expose_event, NULL);
  separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(contenitore1), separator, FALSE, TRUE, 0);
  gtk_widget_show(separator);
  contenitore2 = gtk_hbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(contenitore1), contenitore2, FALSE, FALSE, 0);
  gtk_widget_show(contenitore2);
  for (i = 0; i < 7; i++) {
    bottonecol[i] = gtk_button_new();
    contenitore3 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(contenitore3);
    pixmapwid = gtk_pixmap_new(rombo[i], maschera);
    gtk_widget_show(pixmapwid);
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(contenitore3), pixmapwid, FALSE, FALSE, 0);
    gtk_widget_show(contenitore3);
    gtk_container_add(GTK_CONTAINER(bottonecol[i]), contenitore3);
    gtk_box_pack_start(GTK_BOX(contenitore2), bottonecol[i], TRUE, TRUE, 10);
    gtk_signal_connect(GTK_OBJECT(bottonecol[i]), "clicked",
                       GTK_SIGNAL_FUNC(premuto_colore), (gpointer)i);
    gtk_widget_set_sensitive(bottonecol[i], 0);
    gtk_widget_show(bottonecol[i]);
  }
  separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(contenitore1), separator, FALSE, TRUE, 0);
  gtk_widget_show(separator);
  contenitore2 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(contenitore1), contenitore2, FALSE, FALSE, 0);
  gtk_widget_show(contenitore2);
  bottone = gtk_button_new_with_label("New");
  gtk_box_pack_start(GTK_BOX(contenitore2), bottone, TRUE, FALSE, 10);
  gtk_signal_connect(GTK_OBJECT(bottone), "clicked",
                     GTK_SIGNAL_FUNC(nuovo_click_event), NULL);
  gtk_widget_show(bottone);
  bottone = gtk_button_new_with_label("Exit");
  gtk_box_pack_start(GTK_BOX(contenitore2), bottone, TRUE, FALSE, 10);
  gtk_signal_connect_object(GTK_OBJECT(bottone), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy),
                            GTK_OBJECT(window));
  gtk_widget_show(bottone);
  gtk_main();
}

int leggi_config(int argc, char *argv[]) {
  int i, n;
  altezza_tab = 18;
  larghezza_tab = 18;
  pl[0].type = HUMAN;
  pl[1].type = COMPUTER;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      cmderror();
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
  tab = (struct ttab **)calloc(larghezza_tab, sizeof(struct ttab *));
  for (i = 0; i < larghezza_tab; i++)
    tab[i] = (struct ttab *)calloc(altezza_tab, sizeof(struct ttab));
  srand(time(NULL));
}

void nuovo_gioco(void) {
  int x, y, i;
  short int col;
  for (y = 0; y < altezza_tab; y++) {
    for (x = 0; x < larghezza_tab; x++) {
      col = 7.0 * rand() / (RAND_MAX + 1.0);
      tab[x][y].col = col;
      disegna(x, y, col);
      tab[x][y].segno = 0;
    }
  }
  pl[0].col = tab[0][altezza_tab - 1].col;
  pl[1].col = tab[larghezza_tab - 1][0].col;
  if (pl[1].col == pl[0].col) {
    pl[1].col = (pl[1].col + 1) % 7 + 1;
    tab[larghezza_tab - 1][0].col = pl[1].col;
    disegna(larghezza_tab - 1, 0, pl[1].col);
  }
  pl[0].punti = espandi(0, altezza_tab - 1, pl[0].col);
  scrivi_perc(0);
  pl[1].punti = espandi(larghezza_tab - 1, 0, pl[1].col);
  scrivi_perc(1);
  attivo = 0;
  for (i = 0; i < 7; i++)
    gtk_widget_set_sensitive(bottonecol[i], 1);
  gtk_widget_set_sensitive(bottonecol[pl[0].col], 0);
  gtk_widget_set_sensitive(bottonecol[pl[1].col], 0);
  if (pl[attivo].type == COMPUTER)
    mossa_computer();
}

void disegna(int x, int y, short int col) {
  GdkRectangle update_rect;
  gdk_gc_set_clip_origin(
      miogc, x * larghezza_rombo + ((y + 1) % 2) * larghezza_rombo / 2,
      y * altezza_rombo / 2);
  gdk_draw_pixmap(tavolagioco, miogc, rombo[col], 0, 0,
                  x * larghezza_rombo + ((y + 1) % 2) * larghezza_rombo / 2,
                  y * altezza_rombo / 2, larghezza_rombo, altezza_rombo);
  update_rect.x = x * larghezza_rombo + ((y + 1) % 2) * larghezza_rombo / 2;
  update_rect.y = y * altezza_rombo / 2;
  update_rect.width = larghezza_rombo;
  update_rect.height = altezza_rombo;
  gtk_widget_draw(areadisegno, &update_rect);
}

int colora(int x, int y, short int old_col, short int new_col) {
  int k = 0;
  if (x < larghezza_tab && x >= 0 && y < altezza_tab && y >= 0) {
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
  for (i = 0; i < larghezza_tab; i++)
    for (j = 0; j < altezza_tab; j++)
      tab[i][j].segno = 0;
}

int espandi(int x, int y, short int col) {
  int k = 0;

  if (x < larghezza_tab && x >= 0 && y < altezza_tab && y >= 0) {
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
  if (x < larghezza_tab && x >= 0 && y < altezza_tab && y >= 0) {
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

int riempi(short int col) {
  int x, y, k = 0;
  for (x = 0; x < larghezza_tab; x++) {
    for (y = 0; y < altezza_tab; y++) {
      if (tab[x][y].segno == 0) {
        k++;
        tab[x][y].col = col;
        disegna(x, y, col);
      }
    }
  }
  return k;
}

short int guadmax(short int attivo) {
  short int imax, i;
  int c_max, c;

  c_max = 0;
  imax = 0;
  for (i = 0; i < 7; i++) {
    if (i != pl[(attivo + 1) % 2].col && i != pl[attivo].col) {
      clear();
      c = guadagno(attivo * (larghezza_tab - 1),
                   ((attivo + 1) % 2) * (altezza_tab - 1), pl[attivo].col, i);
      fill(((attivo + 1) % 2) * (larghezza_tab - 1),
           attivo * (altezza_tab - 1));
      c += riempi2();

      if (c >= c_max) {
        c_max = c;
        imax = i;
      }
    }
  }
  return imax;
}

int guadagno(int x, int y, short int old_col, short int new_col) {
  int k = 0;

  if (x < larghezza_tab && x >= 0 && y < altezza_tab && y >= 0) {
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

  for (x = 0; x < larghezza_tab; x++) {
    for (y = 0; y < altezza_tab; y++) {
      if (tab[x][y].segno == 0) {
        k++;
        tab[x][y].segno = 1;
      }
    }
  }
  return k;
}

void scrivi_perc(short int giocatore) {
  char perc[10];

  sprintf(perc, "%03.1f%%",
          pl[giocatore].punti * 100.0 / (larghezza_tab * altezza_tab));
  gtk_label_set_text((GtkLabel *)lblpunti[giocatore], perc);
}

void mossa_computer(void) {
  short int colore;
  colore = guadmax(attivo);
  gtk_widget_set_sensitive(bottonecol[pl[attivo].col], 1);
  gtk_widget_set_sensitive(bottonecol[(int)colore], 0);
  clear();
  pl[attivo].punti = colora(attivo * (larghezza_tab - 1),
                            ((attivo + 1) % 2) * (altezza_tab - 1),
                            pl[attivo].col, (int)colore);
  fill(((attivo + 1) % 2) * (larghezza_tab - 1), attivo * (altezza_tab - 1));
  pl[attivo].punti += riempi((int)colore);
  pl[attivo].col = (int)colore;
  scrivi_perc(attivo);

  if (pl[attivo].punti == larghezza_tab * altezza_tab / 2 &&
      pl[(attivo + 1) % 2].punti == larghezza_tab * altezza_tab / 2)
    gameover(2);
  else if (pl[attivo].punti > larghezza_tab * altezza_tab / 2)
    gameover(attivo);
  else {
    attivo = (attivo + 1) % 2;
    if (pl[attivo].type == COMPUTER)
      mossa_computer();
  }
}

void gameover(int giocatore) {
  int i;

  for (i = 0; i < 7; i++)
    gtk_widget_set_sensitive(bottonecol[i], 0);
}

void cmderror(void) {
  fprintf(stderr, "Usage: 7colors -[1|2] [c|h]\n");
  fprintf(stderr, " -1, -2   player 1 or 2\n");
  fprintf(stderr, " c, h     computer or human\n");
  fprintf(stderr, "Examples:\n");
  fprintf(stderr, "7colors -1 h -2 c\n");
  exit(0);
}
