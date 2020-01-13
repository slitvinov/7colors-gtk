7CFLAGS = `pkg-config gtk+-2.0 --cflags`
7LDFLAGS = `pkg-config gtk+-2.0 --libs`

LDFLAGS =
CFLAGS = -Og -g

7colors: 7colors.c
	$(CC) $(7CFLAGS) $(CFLAGS) $^ $(7LDFLAGS) $(LDFLAGS)-o $@

clean:
	rm -f 7colors
