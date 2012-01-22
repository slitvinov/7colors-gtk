
CC = gcc

7colors: 7colors.c 
	$(CC) -O2 `gtk-config --cflags`  7colors.c -o 7colors `gtk-config --libs`

clean: 
	rm -f *.o 7colors
