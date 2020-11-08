CFLAGS=-I.
INCS=
OBJS=test.c
LIBS=-lssl -lcrypto
TARGET=test
CC=gcc

all: $(TARGET)

%.o: %.c $(INCS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf $(TARGET) *.o