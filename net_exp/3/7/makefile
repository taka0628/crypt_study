TARGET_1 := server
TARGET_2 := client
SORCE_1 := FTserver.c
SORCE_2 := FTclient.c
CC	:= gcc
CFLAGS	:=
RM	:= /bin/rm

all:
	$(CC) $(CFLAGS) $(SORCE_1) -o $(TARGET_1)
	$(CC) $(CFLAGS) $(SORCE_2) -o $(TARGET_2)


$(TARGET_1): $(SORCE_1)
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_2): $(SORCE_2)
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) -rf $(TARGET_1)
	$(RM) -rf $(TARGET_2)
