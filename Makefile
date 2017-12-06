CFLAGS += -Wall -std=c99 -D _BSD_SOURCE

all: remote-kbd sdl-kbd

remote-kbd: remote-kbd.c common.c common.h
	$(CC) remote-kbd.c common.c -o remote-kbd $(CFLAGS)

sdl-kbd: CFLAGS += $(shell pkg-config --cflags sdl2)
sdl-kbd: sdl-kbd.c common.c common.h
	$(CC) sdl-kbd.c common.c -o sdl-kbd $(CFLAGS) $(shell pkg-config --libs sdl2)

clean:
	rm -f remote-kbd sdl-kbd *.o

