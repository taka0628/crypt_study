TARGET_1 := iv_test
SORCE_1 := iv_test.c
CC	:= gcc
CFLAGS	:=-I.
LIBS=-lssl -lcrypto
RM	:= /bin/rm

all:
	$(CC) $(CFLAGS) $(SORCE_1) -o $(TARGET_1) $(LIBS)

$(TARGET_1): $(SORCE_1)
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) -rf $(TARGET_1)
