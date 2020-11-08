# TARGET_RE := receiver
# CC	:= /usr/bin/gcc
# CFLAGS	:= -pthread
# RM	:= /bin/rm

# $(TARGET_RE): mcast_receiver.c
# 	$(CC) $(CFLAGS) $< -o $@

# clean:
# 	$(RM) -rf $(TARGET_RE)

mkaes:
	gcc -pthread mcast_receiver.c -o receiver
	gcc mcast_sender_file.c -o send


# send: mcast_sender_file.c
# 	gcc mcast_sender_file.c -o send
