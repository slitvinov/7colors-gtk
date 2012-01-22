/*
 * 7colors 0.10 - un gioco per XWindow
 *
 * Copyright (C) 2000 Bellotti Stefano <signo@lugbs.linux.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "rombo_grigio.xpm"
#include "rombo_rosso.xpm"
#include "rombo_verde.xpm"
#include "rombo_blu.xpm"
#include "rombo_ciano.xpm"
#include "rombo_magenta.xpm"
#include "rombo_giallo.xpm"


/* un giocatore */
typedef struct {

 /* umano=0 cpu=1 */
 short int tipo;

 /* colore corrente */
 short int col;

 /* numero rombi posseduti */
 int punti;

} tgiocatore;


/* elemento della tavola di gioco */
typedef struct {

 /* Il colore */
 short int col;

 /* per marcare i rombi da cui si e' gia passati */
 short int segno;

} ttab;

/*
static char *nomecolore[]={
 "Grigio",
 "Rosso",
 "Verde",
 "Blu",
 "Ciano",
 "Magenta",
 "Giallo"
}; */

static char *nomecolore[]={
 "Gray",
 "Red",
 "Green",
 "Blue",
 "Cyan",
 "Magenta",
 "Yellow"
};

GtkWidget *areadisegno=NULL;    //area in cui si disegna
GdkPixmap *tavolagioco=NULL;    // pixmap di backup
GdkPixmap *rombo[7];            // i 7 rombi
GdkBitmap *maschera=NULL;       // la maschera per i rombi
GdkGC *miogc=NULL;              // stile per il disegno dei rombi
GtkWidget *lblpunti[2];         // Etichette per i punti
GtkWidget *bottonecol[7];       // Bottoni colorati
GtkWidget *statusbar;

ttab **tab;
tgiocatore pl[2];

int altezza_tab,larghezza_tab,altezza_pixel,larghezza_pixel,
    altezza_rombo,larghezza_rombo;
short int attivo;

void nuovo_gioco(void);
void disegna (int x, int y, short int col);
int colora(int x,int y,short int old_col,short int new_col);
void clear(void);
int espandi(int x,int y,short int col);
void fill(int x,int y);
int riempi(short int col);
int riempi2();
short int guadmax(short int attivo);
int guadagno(int x,int y,short int old_col,short int new_col);
void scrivi_perc(short int giocatore);
void mossa_computer(void);
void gameover(int giocatore);
void cmderror(void);
int leggi_config (int argc, char *argv[]);

/*
 * Richiesta chiusura finestra
 */
gint delete_event (GtkWidget *widget, GdkEvent *event, gpointer data)
{
 return (FALSE);
}

/*
 * Fine programma
 */
void destroy (GtkWidget *widget, gpointer data)
{
 gtk_main_quit();
}

/*
 * Ridisegno parti coperte
 */
static gint expose_event (GtkWidget *widget, GdkEventExpose *event)
{
 gdk_draw_pixmap(widget->window,
                 widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                 tavolagioco,
                 event->area.x, event->area.y,
                 event->area.x, event->area.y,
                 event->area.width, event->area.height);

 return FALSE;
}

/*
 * Cliccato nuovo gioco
 */
void nuovo_click_event (GtkWidget *widget, gpointer data)
{
 //printf("Cliccato nuovo\n");
 nuovo_gioco();
}

/*
 * Cliccato un colore
 */
void premuto_colore (GtkWidget *widget, gpointer colore)
{
  // abilito bottoni
  gtk_widget_set_sensitive(bottonecol[pl[attivo].col],1);
  gtk_widget_set_sensitive(bottonecol[(int)colore],0);
  // tolgo marcature
  clear();
  // coloro, conto e marco i rombi del vecchio colore e quelli adiacenti del
  // nuovo colore
  pl[attivo].punti=colora(attivo*(larghezza_tab-1),
                          ((attivo+1)%2)*(altezza_tab-1),
                          pl[attivo].col,(int)colore);
  // marco tutti i rombi raggiungibili da avversario
  fill(((attivo+1)%2)*(larghezza_tab-1),attivo*(altezza_tab-1));
  // aggiungo i rombi mancanti
  pl[attivo].punti+=riempi((int)colore);
  pl[attivo].col=(int)colore;
  scrivi_perc(attivo);

  if(pl[attivo].punti==larghezza_tab*altezza_tab/2 &&
     pl[(attivo+1)%2].punti==larghezza_tab*altezza_tab/2) gameover(2);  //pari
  else if(pl[attivo].punti>larghezza_tab*altezza_tab/2) gameover(attivo);  //vinto!
  else
  {
   attivo=(attivo+1)%2;
   //vedo se tocca a computer
   if(pl[attivo].tipo==1) mossa_computer();
  }
}

/*
 * Main
 */
int main (int argc, char *argv[])
{
 GtkWidget *window;
 GtkWidget *contenitore1;
 GtkWidget *contenitore2;
 GtkWidget *contenitore3;
 GtkWidget *bottone;
 GtkWidget *separator;
 GtkWidget *pixmapwid;
 GtkWidget *label;
 int i;

 gtk_init (&argc, &argv);

 // Leggo la configurazione e setto i defaults
 leggi_config(argc,argv);

 // la finestra principale
 window=gtk_window_new (GTK_WINDOW_TOPLEVEL);
 gtk_signal_connect (GTK_OBJECT(window), "delete_event",
                     GTK_SIGNAL_FUNC(delete_event), NULL);
 gtk_signal_connect (GTK_OBJECT(window), "destroy",
                     GTK_SIGNAL_FUNC(destroy), NULL);
 gtk_window_set_title (GTK_WINDOW(window), "7Colors");
 gtk_window_set_policy (GTK_WINDOW(window),0,0,0); // no resize
 gtk_container_set_border_width (GTK_CONTAINER (window), 10);
 gtk_widget_show(window);

 // Contenitore verticale grosso
 contenitore1=gtk_vbox_new(FALSE,10);
 gtk_container_add(GTK_CONTAINER(window),contenitore1);
 gtk_widget_show(contenitore1);

 // Contenitore orizzontale per i label punti
 contenitore2=gtk_hbox_new(TRUE,0);
 gtk_box_pack_start(GTK_BOX(contenitore1),contenitore2,FALSE,FALSE,0);
 gtk_widget_show(contenitore2);

 // Label Punti giocatore 1
 lblpunti[0]=gtk_label_new("0\%");
 gtk_box_pack_start(GTK_BOX(contenitore2),lblpunti[0],
                    TRUE,TRUE,0);
 //gtk_label_set_justify(GTK_LABEL(lblpunti[0]),GTK_JUSTIFY_LEFT);
 gtk_widget_show(lblpunti[0]);

 // Label Punti giocatore 2
 lblpunti[1]=gtk_label_new("0\%");
 gtk_box_pack_start(GTK_BOX(contenitore2),lblpunti[1],
                    TRUE,TRUE,0);
 //gtk_label_set_justify(GTK_LABEL(lblpunti[1]),GTK_JUSTIFY_RIGHT);
 gtk_widget_show(lblpunti[1]);

 // Areadisegno: dove tutto viene disegnato
 areadisegno=gtk_drawing_area_new();
 gtk_box_pack_start(GTK_BOX(contenitore1),areadisegno,FALSE,FALSE,0);
 gtk_widget_show(areadisegno);

 // creo gli xpm
 rombo[0]=gdk_pixmap_create_from_xpm_d (areadisegno->window, &maschera,
          &areadisegno->style->bg[GTK_STATE_NORMAL],(gchar **)rombo_grigio_xpm);
 rombo[1]=gdk_pixmap_create_from_xpm_d (areadisegno->window, &maschera,
          &areadisegno->style->bg[GTK_STATE_NORMAL],(gchar **)rombo_rosso_xpm);
 rombo[2]=gdk_pixmap_create_from_xpm_d (areadisegno->window, &maschera,
          &areadisegno->style->bg[GTK_STATE_NORMAL],(gchar **)rombo_verde_xpm);
 rombo[3]=gdk_pixmap_create_from_xpm_d (areadisegno->window, &maschera,
          &areadisegno->style->bg[GTK_STATE_NORMAL],(gchar **)rombo_blu_xpm);
 rombo[4]=gdk_pixmap_create_from_xpm_d (areadisegno->window, &maschera,
          &areadisegno->style->bg[GTK_STATE_NORMAL],(gchar **)rombo_ciano_xpm);
 rombo[5]=gdk_pixmap_create_from_xpm_d (areadisegno->window, &maschera,
          &areadisegno->style->bg[GTK_STATE_NORMAL],(gchar **)rombo_magenta_xpm);
 rombo[6]=gdk_pixmap_create_from_xpm_d (areadisegno->window, &maschera,
          &areadisegno->style->bg[GTK_STATE_NORMAL],(gchar **)rombo_giallo_xpm);

 // creo gc per disegnare trasparente
 miogc=gdk_gc_new (areadisegno->window);
 gdk_gc_set_clip_mask(miogc, maschera);

 // leggo dimensioni xpm
 gdk_window_get_size(maschera,&larghezza_rombo,&altezza_rombo);
 altezza_pixel=altezza_tab*altezza_rombo/2+altezza_rombo/2;
 larghezza_pixel=larghezza_tab*larghezza_rombo+larghezza_rombo/2;

 // Dimensiono l'area disegno
 gtk_drawing_area_size (GTK_DRAWING_AREA(areadisegno),
                        larghezza_pixel,altezza_pixel);

 // Creo la pixmap di backup
 tavolagioco=gdk_pixmap_new(areadisegno->window,
                            larghezza_pixel,altezza_pixel,-1);

 // Pulisco la pixmap di backup
 gdk_draw_rectangle (tavolagioco,window->style->black_gc,
                     TRUE,0,0,larghezza_pixel,altezza_pixel);

 // Attivo evento di refresh
 gtk_signal_connect (GTK_OBJECT (areadisegno), "expose_event",
                     (GtkSignalFunc) expose_event, NULL);

 // Linea separazione
 separator = gtk_hseparator_new ();
 gtk_box_pack_start(GTK_BOX(contenitore1),separator,FALSE,TRUE,0);
 gtk_widget_show(separator);

 // Contenitore orizzontale per bottoni colore
 contenitore2=gtk_hbox_new(TRUE,0);
 gtk_box_pack_start(GTK_BOX(contenitore1),contenitore2,FALSE,FALSE,0);
 gtk_widget_show(contenitore2);

 // bottoni colorati
 for(i=0;i<7;i++)
 {
  bottonecol[i]=gtk_button_new();
  contenitore3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show(contenitore3);
  pixmapwid = gtk_pixmap_new (rombo[i], maschera);
  gtk_widget_show(pixmapwid);
  label = gtk_label_new (nomecolore[i]);
  gtk_widget_show(label);
  gtk_box_pack_start (GTK_BOX (contenitore3), pixmapwid, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (contenitore3), label, FALSE, FALSE, 0);
  gtk_widget_show(contenitore3);
  gtk_container_add (GTK_CONTAINER (bottonecol[i]), contenitore3);
  gtk_box_pack_start(GTK_BOX(contenitore2),bottonecol[i],TRUE,TRUE,10);
  gtk_signal_connect(GTK_OBJECT(bottonecol[i]), "clicked",
                     GTK_SIGNAL_FUNC(premuto_colore), (gpointer)i);
  gtk_widget_set_sensitive(bottonecol[i],0);
  gtk_widget_show(bottonecol[i]);
 }

 // Un'alra barra di separazione
 separator = gtk_hseparator_new ();
 gtk_box_pack_start(GTK_BOX(contenitore1),separator,FALSE,TRUE,0);
 gtk_widget_show(separator);

 // Contenitore orizzontale per i bottoni 'nuovo' e 'esci'
 contenitore2=gtk_hbox_new(FALSE,0);
 gtk_box_pack_start(GTK_BOX(contenitore1),contenitore2,FALSE,FALSE,0);
 gtk_widget_show(contenitore2);

 // Bottone 'Nuovo'
 bottone=gtk_button_new_with_label("New");
 gtk_box_pack_start(GTK_BOX(contenitore2),bottone,TRUE,FALSE,10);
 gtk_signal_connect (GTK_OBJECT (bottone), "clicked",
                     GTK_SIGNAL_FUNC (nuovo_click_event),
                     NULL);
 gtk_widget_show(bottone);

 // Bottone 'Esci'
 bottone=gtk_button_new_with_label("Exit");
 gtk_box_pack_start(GTK_BOX(contenitore2),bottone,TRUE,FALSE,10);
 gtk_signal_connect_object (GTK_OBJECT (bottone), "clicked",
                            GTK_SIGNAL_FUNC (gtk_widget_destroy),
                            GTK_OBJECT (window));
 gtk_widget_show(bottone);

 // Inizializzo un nuovo gioco
 //nuovo_gioco();

 gtk_main ();

 return (0);
}


/*
 * Legge il file di configurazione, la linea di comando e configura le strutture
 */
int leggi_config (int argc, char *argv[])
{
 int i,n;

 altezza_tab=18;
 larghezza_tab=18;
 pl[0].tipo=0;
 pl[1].tipo=1;

 n=1;
 while(n<argc)
 {
  if(strcmp(argv[n],"-1")==0 || strcmp(argv[n],"-2")==0)
  {
   if(strcmp(argv[n],"-1")==0) i=0;
   else i=1;
   n++;
   if(n>=argc) cmderror();
   if(strcmp(argv[n],"h")==0) pl[i].tipo=0;
   else if(strcmp(argv[n],"c")==0) pl[i].tipo=1;
   else cmderror();
  }
  else cmderror();
  n++;
 }

 tab=(ttab**)calloc(larghezza_tab,sizeof(ttab*));
 for(i=0;i<larghezza_tab;i++) tab[i]=(ttab*)calloc(altezza_tab,sizeof(ttab));

 srand(time(NULL));
 return 0;
}

/*
 * Inizializza e disegna casualmente una nuova tavola di gioco
 */
void nuovo_gioco(void)
{
 int x,y,i;
 short int col;

 //inizializzo tab
 for(y=0;y<altezza_tab;y++)
 {
  for(x=0;x<larghezza_tab;x++)
  {
   col=7.0*rand()/(RAND_MAX+1.0);
   tab[x][y].col=col;
   disegna(x,y,col);
   tab[x][y].segno=0;
  }
 }
 // colori dei giocatori
 pl[0].col=tab[0][altezza_tab-1].col;
 pl[1].col=tab[larghezza_tab-1][0].col;
 //se colori uguali
 if(pl[1].col==pl[0].col)
 {
  pl[1].col=(pl[1].col+1)%7+1;
  tab[larghezza_tab-1][0].col=pl[1].col;
  disegna(larghezza_tab-1,0,pl[1].col);
 }
 //conteggio punti
 pl[0].punti=espandi(0,altezza_tab-1,pl[0].col);
 scrivi_perc(0);
 pl[1].punti=espandi(larghezza_tab-1,0,pl[1].col);
 scrivi_perc(1);

 //giocatore attivo
 attivo=0;

 // abilitazione bottoni
 for(i=0;i<7;i++) gtk_widget_set_sensitive(bottonecol[i],1);
 gtk_widget_set_sensitive(bottonecol[pl[0].col],0);
 gtk_widget_set_sensitive(bottonecol[pl[1].col],0);

 //vedo se tocca a computer
 if(pl[attivo].tipo==1) mossa_computer();
}

/*
 * Disegna sullo schermo il rombo
 */
void disegna (int x, int y, short int col)
{
 GdkRectangle update_rect;

 gdk_gc_set_clip_origin (miogc,
                         x*larghezza_rombo+((y+1)%2)*larghezza_rombo/2,
                         y*altezza_rombo/2);
 gdk_draw_pixmap (tavolagioco, miogc, rombo[col], 0, 0,
                  x*larghezza_rombo+((y+1)%2)*larghezza_rombo/2,
                  y*altezza_rombo/2, larghezza_rombo, altezza_rombo);

 // Definisco area da ridisegnare
 update_rect.x = x*larghezza_rombo+((y+1)%2)*larghezza_rombo/2;
 update_rect.y = y*altezza_rombo/2;
 update_rect.width =larghezza_rombo;
 update_rect.height = altezza_rombo;

 // refresh
 gtk_widget_draw (areadisegno, &update_rect);

}


/*
 * Funzione ricorsiva che sostituisce i rombi di colore old_col con new_col.
 * Quando vicino al rombo old_col c'e' un rombo new_col da cui non si e'
 * ancora passati viene chiamata la funzione ricorsiva espandi.
 * Ritorna il numero totale di rombi acquisiti.
 */
int colora(int x,int y,short int old_col,short int new_col)
{
 int k=0;

 if(x<larghezza_tab && x>=0 && y<altezza_tab && y>=0)
 {
  if(tab[x][y].col==old_col)
  {
   tab[x][y].col=new_col;
   disegna(x,y,new_col);
   tab[x][y].segno=1;
   if(y%2==0)
    k=colora(x,y-1,old_col,new_col)+colora(x,y+1,old_col,new_col)+
      colora(x+1,y-1,old_col,new_col)+colora(x+1,y+1,old_col,new_col)+1;
   else
    k=colora(x,y-1,old_col,new_col)+colora(x,y+1,old_col,new_col)+
    colora(x-1,y-1,old_col,new_col)+colora(x-1,y+1,old_col,new_col)+1;
  }
  else
  {
   if(tab[x][y].col==new_col && tab[x][y].segno==0)
   {
    tab[x][y].segno=1;
    if(y%2==0)
     k=espandi(x,y-1,new_col)+espandi(x,y+1,new_col)+
       espandi(x+1,y-1,new_col)+espandi(x+1,y+1,new_col)+1;
    else
     k=espandi(x,y-1,new_col)+espandi(x,y+1,new_col)+
     espandi(x-1,y-1,new_col)+espandi(x-1,y+1,new_col)+1;
   }
  }
 }
 return k;
}

/*
 * Resetta i campi segno
 */
void clear(void)
{
 int i,j;

 for(i=0;i<larghezza_tab;i++) for(j=0;j<altezza_tab;j++) tab[i][j].segno=0;
}

/*
 * Funzione ricorsiva che visita e marca tutti i rombi di colore col
 * adiacenti a quallo di partenza.
 * Ritorna il numero totale di rombi visitati.
 */
int espandi(int x,int y,short int col)
{
 int k=0;

 if(x<larghezza_tab && x>=0 && y<altezza_tab && y>=0)
 {
  if(tab[x][y].col==col && tab[x][y].segno==0)
  {
   tab[x][y].segno=1;
   if(y%2==0)
    k=espandi(x,y-1,col)+espandi(x,y+1,col)+
      espandi(x+1,y-1,col)+espandi(x+1,y+1,col)+1;
   else
    k=espandi(x,y-1,col)+espandi(x,y+1,col)+
      espandi(x-1,y-1,col)+espandi(x-1,y+1,col)+1;
  }
 }
 return k;
}

/*
 * Funzione ricorsiva che marca tutti i rombi raggiungibili dalla posizione
 * x,y senza passare su robi gia' marcati.
 */
void fill(int x,int y)
{
 if(x<larghezza_tab && x>=0 && y<altezza_tab && y>=0)
 {
  if(tab[x][y].segno==0)
  {
   tab[x][y].segno=1;
   if(y%2==0) {fill(x,y-1);fill(x,y+1);fill(x+1,y-1);fill(x+1,y+1);}
   else {fill(x,y-1);fill(x,y+1);fill(x-1,y-1);fill(x-1,y+1);}
  }
 }
}


/*
 * Colora tutti i rombi non marcati.
 * Restituisce il numero di rombi colorati.
 */
int riempi (short int col)
{
 int x,y,k=0;

 for(x=0;x<larghezza_tab;x++)
 {
  for(y=0;y<altezza_tab;y++)
  {
   if(tab[x][y].segno==0)
   {
    k++;
    tab[x][y].col=col;
    disegna(x,y,col);
   }
  }
 }
 return k;
}


/*
 * prova tutti i colori e trova quello che conquista il maggior numero di
 * rombi
 */
short int guadmax(short int attivo)
{
 short int imax,i;
 int c_max,c;

 c_max=0;
 imax=0;
 for(i=0;i<7;i++)
 {
  if(i!=pl[(attivo+1)%2].col && i!=pl[attivo].col)
  {
   clear();
   c=guadagno(attivo*(larghezza_tab-1), ((attivo+1)%2)*(altezza_tab-1),
              pl[attivo].col,i);
   fill(((attivo+1)%2)*(larghezza_tab-1),attivo*(altezza_tab-1));
   c+=riempi2();

   if(c>=c_max)
   {
    c_max=c;
    imax=i;
   }
  }
 }
 return imax;
}


/*
 * come la funz colora, ma invece di colorare i pezzi di colore old_col,
 * li marca soltanto
 */
int guadagno(int x,int y,short int old_col,short int new_col)
{
 int k=0;

 if(x<larghezza_tab && x>=0 && y<altezza_tab && y>=0)
 {
  if(tab[x][y].col==old_col && tab[x][y].segno==0)
  {
   tab[x][y].segno=1;
   if(y%2==0)
    k=guadagno(x,y-1,old_col,new_col)+guadagno(x,y+1,old_col,new_col)+
      guadagno(x+1,y-1,old_col,new_col)+guadagno(x+1,y+1,old_col,new_col)+1;
   else
    k=guadagno(x,y-1,old_col,new_col)+guadagno(x,y+1,old_col,new_col)+
      guadagno(x-1,y-1,old_col,new_col)+guadagno(x-1,y+1,old_col,new_col)+1;
  }
  else
  {
   if(tab[x][y].col==new_col && tab[x][y].segno==0)
   {
    tab[x][y].segno=1;
    if(y%2==0)
     k=espandi(x,y-1,new_col)+espandi(x,y+1,new_col)+
       espandi(x+1,y-1,new_col)+espandi(x+1,y+1,new_col)+1;
    else
     k=espandi(x,y-1,new_col)+espandi(x,y+1,new_col)+
     espandi(x-1,y-1,new_col)+espandi(x-1,y+1,new_col)+1;
   }
  }
 }
 return k;
}


/*
 * Come riempi, ma non colora i rombi, li marca soltanto.
 * Restituisce il numero di rombi marcati.
 */
int riempi2()
{
 int x,y,k=0;

 for(x=0;x<larghezza_tab;x++)
 {
  for(y=0;y<altezza_tab;y++)
  {
   if(tab[x][y].segno==0)
   {
    k++;
    tab[x][y].segno=1;
   }
  }
 }
 return k;
}


/*
 * Scrive la percentuale a video
 */
void scrivi_perc(short int giocatore)
{
 char perc[10];

 sprintf(perc,"%03.1f%%",pl[giocatore].punti*100.0/(larghezza_tab*altezza_tab));
 gtk_label_set_text((GtkLabel *)lblpunti[giocatore],perc);
}


/*
 * Il computer fa una mossa
 */
void mossa_computer(void)
{
 short int colore;

 //prendo il colore migliore
 colore=guadmax(attivo);

 // abilito bottoni
 gtk_widget_set_sensitive(bottonecol[pl[attivo].col],1);
 gtk_widget_set_sensitive(bottonecol[(int)colore],0);
 // tolgo marcature
 clear();
 // coloro, conto e marco i rombi del vecchio colore e quelli adiacenti del
 // nuovo colore
 pl[attivo].punti=colora(attivo*(larghezza_tab-1),
                         ((attivo+1)%2)*(altezza_tab-1),
                         pl[attivo].col,(int)colore);
 // marco tutti i rombi raggiungibili da avversario
 fill(((attivo+1)%2)*(larghezza_tab-1),attivo*(altezza_tab-1));
 // aggiungo i rombi mancanti
 pl[attivo].punti+=riempi((int)colore);
 pl[attivo].col=(int)colore;
 scrivi_perc(attivo);

 if(pl[attivo].punti==larghezza_tab*altezza_tab/2 &&
    pl[(attivo+1)%2].punti==larghezza_tab*altezza_tab/2) gameover(2);  //pari
 else if(pl[attivo].punti>larghezza_tab*altezza_tab/2) gameover(attivo); //vinto!
 else
 {
  attivo=(attivo+1)%2;
  //vedo se tocca a computer
  if(pl[attivo].tipo==1) mossa_computer();
 }
}


/*
 * Fine gioco
 */
void gameover(int giocatore)
{
 int i;

 for(i=0; i<7; i++) gtk_widget_set_sensitive(bottonecol[i],0);

}

/*
 * Errore su linea comando
 */
void cmderror(void)
{
 /*
 printf("Opzioni errate\n");
 printf("Utilizzo: 7colors -[1|2] [c|h]\n");
 printf(" -1, -2   giocatore 1 o 2\n");
 printf(" c, h     computer o umano\n");
 exit(0);
 */
 printf("Invalid options\n");
 printf("Usage: 7colors -[1|2] [c|h]\n");
 printf(" -1, -2   player 1 or 2\n");
 printf(" c, h     computer or human\n");
 exit(0);

}
