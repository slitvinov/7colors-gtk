# Intro

[7colors](https://en.wikipedia.org/wiki/7_Colors) game. See [src/README](src/README).

<p align="center"><img src="img/screen.png"/></p>

# Install

Requires GTK 2. To install on debian:

```
$ apt install libgtk2.0-dev
```

Build and run

```
$ (cd src && make)
$ src/gtk -h
Usage: 7colors -[1|2] [c|h]
 -1, -2   player 1 or 2
 c, h     computer or human
Examples:
7colors -1 h -2 c
```
