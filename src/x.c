#include "blu.xpm"
#include "ciano.xpm"
#include "giallo.xpm"
#include "grigio.xpm"
#include "magenta.xpm"
#include "rosso.xpm"
#include "verde.xpm"
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

int main(int argc, char *argv[]) {
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
}
