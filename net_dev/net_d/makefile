TARGET_1 := server
SORCE_1 := server.c
CC	:= gcc
CFLAGS	:=
RM	:= /bin/rm

all:
	$(CC) $(CFLAGS) $(SORCE_1) -o $(TARGET_1)


$(TARGET_1): $(SORCE_1)
	$(CC) $(CFLAGS) $< -o $@
clean:
	$(RM) -rf $(TARGET_1)
