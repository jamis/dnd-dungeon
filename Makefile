all: cgi console

cgi: dungeon.cgi

console: dungeon

CC=gcc
CPP=g++

LIBS=$(LDFLAGS) -lm -lqDecoder -lgd -lpng -lz -ldndutil -lnpcEngine -lwritetem

OPTS=$(CFLAGS) -Iinclude -O3 -Wall -DUSE_COUNTER -DCTRLOCATION="\"/tmp/dungeon.cnt\""

.SUFFIXES:

.SUFFIXES: .cpp .c .o

.cpp.o:
	$(CPP) $(OPTS) -c -o $@ $<

.c.o:
	$(CC) $(OPTS) -c -o $@ $<

OBJS=\
	src/jbdungeon.o \
	src/jbdungeondata.o \
	src/jbdungeonpainter.o \
	src/jbdungeonpaintergd.o \
	src/jbmaze.o \
	src/jbmazemask.o \
	src/treasureEngine.o

dungeon.cgi: src/dungeoncgi.o $(OBJS)
	$(CPP) $(OPTS) -o dungeon.cgi src/dungeoncgi.o $(OBJS) $(LIBS)

dungeon: src/main.o $(OBJS)
	$(CPP) $(OPTS) -o dungeon src/main.o $(OBJS) $(LIBS)

clean:
	rm -f src/*.o
	rm -f dungeon.cgi
	rm -f dungeon
