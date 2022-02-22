CC = gcc
CFLAGS = -Wall -O3 
INCLUDES =
LFLAGS = 
LIBS = -ljpeg -lm 

OBJS = max2sphere.o bitmaplib.o

all: max2sphere

max2sphere: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o max2sphere $(OBJS) $(LFLAGS) $(LIBS)

max2sphere.o: max2sphere.c max2sphere.h
	$(CC) $(INCLUDES) $(CFLAGS) -c max2sphere.c
 
bitmaplib.o: bitmaplib.c bitmaplib.h
	$(CC) $(INCLUDES) $(CFLAGS) -c bitmaplib.c

clean:
	rm -rf core max2sphere $(OBJS) 

