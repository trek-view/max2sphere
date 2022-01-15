CC = gcc
CFLAGS = -Wall -O3 
INCLUDES =
LFLAGS = 
LIBS = -ljpeg -lm 

OBJS = MAX2spherebatch.o bitmaplib.o

all: MAX2spherebatch

MAX2spherebatch: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o MAX2spherebatch $(OBJS) $(LFLAGS) $(LIBS)

MAX2spherebatch.o: MAX2spherebatch.c MAX2spherebatch.h
	$(CC) $(INCLUDES) $(CFLAGS) -c MAX2spherebatch.c
 
bitmaplib.o: bitmaplib.c bitmaplib.h
	$(CC) $(INCLUDES) $(CFLAGS) -c bitmaplib.c

clean:
	rm -rf core MAX2spherebatch $(OBJS) 

