COMMON = fdlogo.o fdface.o
CLOCKOBJS = fdclock.o fdhand.o findargb.o $(COMMON)
PNGOBJS = fdfacepng.o write_png.o $(COMMON)
INCLUDES=`pkg-config --cflags cairo`
CLOCKLIBS= `pkg-config --libs cairo`
PNGLIBS=`pkg-config --libs cairo` -lpng
CFLAGS=-g

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $*.c 

all: fdclock fdfacepng

fdclock: $(CLOCKOBJS)
	$(CC) $(CFLAGS) -o $@ $(CLOCKOBJS) $(CLOCKLIBS)

fdfacepng: $(PNGOBJS)
	$(CC) $(CFLAGS) -o $@ $(PNGOBJS) $(PNGLIBS)

clean:
	rm -f fdclock $(OBJS)
