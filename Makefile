CC=gcc
CFLAGS=-I"C:/msys64/mingw64/include"
LDFLAGS=-L"C:/msys64/mingw64/lib"
LIBS=-luser32 -lgdi32 -lcurl -lpng -lz -lrpcrt4

logger: logger.c
	$(CC) $(CFLAGS) $(LDFLAGS) logger.c -o logger  $(LIBS)
