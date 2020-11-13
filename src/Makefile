.POSIX:
.SUFFIXES:
.SUFFIXES: .c

7FLAGS = `pkg-config gtk+-2.0 --cflags --libs`
CFLAGS = -Og -g
all: 7colors
.c:; $(CC) $< $(CFLAGS) $(7FLAGS) $(LDFLAGS) -o $@
clean:; -rm 7colors

7colors: blu.xpm
7colors: ciano.xpm
7colors: giallo.xpm
7colors: grigio.xpm
7colors: magenta.xpm
7colors: rosso.xpm
7colors: verde.xpm
