#
# Makefile for CheeZeBoT
#
# (c) 1997 Eric Robbins
#

CC = gcc
LIBS =
CFLAGS = -g3
PROG = cheeze
DEFINES = -DDEBUG

OBJ = channel.o setup.o parse.o aux.o user.o match.o commands.o \
	dcc.o cheeze.o
SRC = channel.c setup.c parse.c aux.c user.c match.c commands.c \
	dcc.c cheeze.c

.c.o: cheeze.h commands.h struct.h Makefile
	$(CC) $(CFLAGS) $(DEFINES) -c $<

all: $(OBJ) Makefile
	$(CC) $(CFLAGS) $(DEFINES) -o $(PROG) $(OBJ) $(LIBS)

obj: $(OBJ) Makefile

clean:
	rm -f $(OBJ) $(PROG) $(PROG).core core
