TARGET_1 := iv_test
SORCE_1 := iv_test.c
CC	:= gcc
CFLAGS	:= -Wall
LIBS=-lssl -lcrypto -I.
RM	:= /bin/rm

all:
	$(CC) $(CFLAGS) $(SORCE_1) -o $(TARGET_1) $(LIBS)

$(TARGET_1): $(SORCE_1)
	$(CC) $(CFLAGS) $< -o $@

debug:
	$(CC) $(SORCE_1) -g -O0 -o $(TARGET_1) $(LIBS)

clean:
	$(RM) -rf $(TARGET_1)
