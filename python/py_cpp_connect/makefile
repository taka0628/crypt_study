# boostラッパー用makefile

TARGET := CModule.so
OBJECTS += wrap_file.o
CC	:= g++
CFLAGS	:= \
	-Wall\
	-fPIC \
	-shared\
	-DPIC\
	-lpython3.6m
LIBS := \
	-I /usr/include/python3.6m
RM	:= /bin/rm

all:${OBJECTS}
	$(CC) ${LIBS} ${CFLAGS} -o ${TARGET} ${OBJECTS}

%.o: %.cpp
	$(CC) ${CFLAGS} -c $< -o $@ ${LIBS}

run: 
	./$(TARGET)

debug:
	$(CC) -g -Wall -O0 main.cpp mem.cpp RSA.cpp -o ${TARGET} ${LIBS}

clean:
	rm -rf ${OBJECTS} ${TARGET}