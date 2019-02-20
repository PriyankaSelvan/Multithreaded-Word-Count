CC = gcc
IDIR = include
SDIR = src
src = $(wildcard src/*.c)
obj = $(src:.c=.o)
CFLAGS=-I$(IDIR)
LDFLAGS = -lpthread

ssfi: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

