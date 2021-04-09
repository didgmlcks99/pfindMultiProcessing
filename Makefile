#Makefile

CC = gcc
CFLAGS = -W -Wall
TARGET = pfind
DTARGET = pfind_debug
OBJECTS = pfind.c
all = $(TARGET)
$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
$(DTARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -DDEBUG -o $@ $^
clean :
	rm pfind pfind_debug
