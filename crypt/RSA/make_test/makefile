TARGET := main
OBJECTS += main.o mem.o RSA.o 
CC	:= g++
CFLAGS	:= -c -Wall -O1
LIBS := -lssl -lcrypto -I.
RM	:= /bin/rm

all:${OBJECTS}
	$(CC) ${OBJECTS} -o ${TARGET} ${LIBS}

%.o: %.cpp
	$(CC) ${CFLAGS} $< -o $@ $(LIBS)

run: 
	./$(TARGET)

debug:
	$(CC) -g -Wall -O0 main.cpp mem.cpp RSA.cpp -o ${TARGET} ${LIBS}

clean:
	rm -rf ${OBJECTS} ${TARGET}