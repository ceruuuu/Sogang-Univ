CC = gcc
TARGET = testlib
OBJECTS = debug.o hex_dump.o list.o hash.o bitmap.o main.o

$(TARGET) : $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS)

clean :
	rm -rf *.o testlib
