PROGNAME = remote-kbd
CFLAGS = -Wall -std=c99 -D _BSD_SOURCE
LDFLAGS =

all: $(PROGNAME)

$(PROGNAME): $(PROGNAME).c
	$(CC) $(PROGNAME).c -o $(PROGNAME) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(PROGNAME) *.o

