.POSIX:
.SUFFIXES:

7FLAGS = `pkg-config gtk+-2.0 --cflags --libs`
XCBFLAGS = `pkg-config --cflags --libs xcb`
CFLAGS = -Og -g
all: gtk
gtk: gtk.c; $(CC) $< $(CFLAGS) $(7FLAGS) $(LDFLAGS) -o $@
x: x.c; $(CC) $< $(CFLAGS) $(XCBFLAGS) $(LDFLAGS) -o $@
clean:; -rm gtk x
gtk: blu.xpm
gtk: ciano.xpm
gtk: giallo.xpm
gtk: grigio.xpm
gtk: magenta.xpm
gtk: rosso.xpm
gtk: verde.xpm
