.PHONY: all clean directories

# Disable implicit rules
.SUFFIXES:

.PRECIOUS: %.o


# Compiler and linker options
CC = gcc
CFLAGS = -Wall -Werror -g
LDFLAGS =
LIBS += -lpthread

# Include directory and header file
INCLUDE = csapp.h protocol.h
INCLDIR = -I.

# Object files and programs
OBJS = csapp.o protocol.o
PROGS = client server master

# Output directories
CLIENT_DIR = ../out/client
SERVER_DIR = ../out/servers

# Targets
all: directories $(PROGS) move config

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
	mv master $(SERVER_DIR)
	mv server $(SERVER_DIR)

config:
	touch $(SERVER_DIR)/config.txt
# Clean up
clean:
	rm -rf $(CLIENT_DIR) $(SERVER_DIR) $(OBJS)
