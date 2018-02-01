OS := $(shell uname -s)

# Variables
CFLAGS  := -fpermissive -g -Wall `root-config --cflags` -I.
LFLAGS  := `root-config --libs`

ifeq ($(OS),Linux) #hack to make this compile on OS X
	LFLAGS += -lrt
endif

CC      := g++
GCC      := gcc
BIN     := $(PWD)

# Root Sources
ROOT_DIR := $(PWD)
ROOT_SRC := $(wildcard $(ROOT_DIR)/*.cpp)
ROOT_BIN := $(patsubst $(ROOT_DIR)/%.cpp,$(BIN)/%,$(ROOT_SRC))
#ROOT_OBJ := $(OBJ)/meeg_utils.o $(OBJ)/cosmic_utils.o

# Default
all: $(ROOT_BIN)

# Object directory
clean:
	rm $(ROOT_BIN)

## Compile ROOT Sources
#$(OBJ)/%.o: $(ROOT_DIR)/%.cc $(ROOT_DIR)/%.hh
	#$(CC) -c $(CFLAGS) $(DEF) -o $@ $<

# Compile root
$(BIN)/%: $(ROOT_DIR)/%.cpp
	$(CC) $(CFLAGS) $(DEF) -o $@ $< $(LFLAGS)

