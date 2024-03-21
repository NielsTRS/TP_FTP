.PHONY: all clean directories

# Disable implicit rules
.SUFFIXES:

# Compiler and linker options
CC = gcc
CFLAGS = -Wall -Werror
LDFLAGS =
LIBS += -lpthread

# Include directory and header file
INCLUDE = csapp.h
INCLDIR = -I.

# Object files and programs
OBJS = csapp.o
PROGS = client server

# Output directories
CLIENT_DIR = ../out/client
SERVER_DIR = ../out/server

# Targets
all: directories $(PROGS) move

directories:
	mkdir -p $(CLIENT_DIR) $(SERVER_DIR)

# Compilation rule for object files
%.o: %.c $(INCLUDE)
	$(CC) $(CFLAGS) $(INCLDIR) -c -o $@ $<

# Linking rule for executables
%: %.o $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

# Move executables to output directories
move:
	mv client $(CLIENT_DIR)
	mv server $(SERVER_DIR)

# Clean up
clean:
	rm -f $(CLIENT_DIR)/client $(SERVER_DIR)/server $(OBJS)
