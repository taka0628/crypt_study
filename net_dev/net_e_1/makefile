TARGET_1 := server
SORCE_1 := server.c
CC	:= gcc
CFLAGS	:= -pthread
RM	:= /bin/rm

all:
	$(CC) $(CFLAGS) $(SORCE_1) -o $(TARGET_1)


$(TARGET_1): $(SORCE_1)
	$(CC) $(CFLAGS) $< -o $@
	
wall:
	$(CC) $(CFLAGS) -Wall $(SORCE_1) -o $(TARGET_1)

clean:
	$(RM) -rf $(TARGET_1)
