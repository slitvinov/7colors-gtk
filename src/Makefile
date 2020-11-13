.POSIX:
.SUFFIXES:
.SUFFIXES: .c

7FLAGS = `pkg-config gtk+-2.0 --cflags --libs`
CFLAGS = -Og -g
LINK = $(CC)

all: 7colors
.c:; $(LINK) $< $(7FLAGS) $(LDFLAGS) -o $@
clean:; -rm 7colors
