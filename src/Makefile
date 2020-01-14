.POSIX:
.PHONY: clean all

7CFLAGS = `pkg-config gtk+-2.0 --cflags`
7LDFLAGS = `pkg-config gtk+-2.0 --libs`

LDFLAGS =
CFLAGS = -Og -g
LINK = $(CC)

all: 7colors
.o:; $(LINK) $< $(LDFLAGS) $(7LDFLAGS) -o $@
.c.o:; $(CC) $(CFLAGS) $(7CFLAGS) $< -c

.c:
%: %.c

clean:; rm -f 7colors 7colors.o


