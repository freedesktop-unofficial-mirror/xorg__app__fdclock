OBJS = fdclock.o write_png.o
INCLUDES=`pkg-config --cflags cairo`
LIBS= `pkg-config --libs cairo` -lpng
CFLAGS=-g

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $*.c 
	
fdclock: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f fdclock $(OBJS)
