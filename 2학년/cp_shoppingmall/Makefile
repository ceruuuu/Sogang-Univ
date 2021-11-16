CC = gcc
CFLAGS = -W -Wall
TARGET = cp_project
OBJECTS = cp_project.o

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean :
	rm -rf $(OBJECTS) $(TARGET)
