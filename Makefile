# $Id: Makefile,v 1.2 1998/09/13 00:21:18 emanuel Exp $

CFLAGS = -O
LDFLAGS =
LIBS = -lm

SRCS = main.c error.c \
       wave.c aiff.c riff.c extended.c \
       fftn.c fftaux.c
OBJS = main.o error.o \
       wave.o aiff.o riff.o extended.o \
       fftn.o fftaux.o
OUT = vocoder

$(OUT): $(OBJS)
	$(CC) $(LDFLAGS) -o $(OUT) $(OBJS) $(LIBS)

depend: 
	makedepend -- $(CFLAGS) -- $(SRCS)

clean:
	rm -f $(OUT) *.o *~ *.bak *.aif *.aiff *.wav *core
