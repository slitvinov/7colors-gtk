.POSIX:
.SUFFIXES:

7FLAGS = `pkg-config gtk+-2.0 --cflags --libs`
CFLAGS = -Og -g
all: gtk x
gtk: gtk.c; $(CC) $< $(CFLAGS) $(7FLAGS) $(LDFLAGS) -o $@
x: x.c; $(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
clean:; -rm gtk x
gtk: blu.xpm
gtk: ciano.xpm
gtk: giallo.xpm
gtk: grigio.xpm
gtk: magenta.xpm
gtk: rosso.xpm
gtk: verde.xpm
x: blu.xpm
x: ciano.xpm
x: giallo.xpm
x: grigio.xpm
x: magenta.xpm
x: rosso.xpm
x: verde.xpm
