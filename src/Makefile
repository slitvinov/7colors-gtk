.POSIX:
.SUFFIXES:
.SUFFIXES: .c

7FLAGS = `pkg-config gtk+-2.0 --cflags --libs`
CFLAGS = -Og -g
all: 7colors
.c:; $(CC) $< $(CFLAGS) $(7FLAGS) $(LDFLAGS) -o $@
clean:; -rm 7colors
