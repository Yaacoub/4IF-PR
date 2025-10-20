# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -O2 -g
LDFLAGS :=
RM := rm -f

# Directories
SRC_DIR := src
OBJ_DIR := build

# Files
SOURCES := $(shell find $(SRC_DIR) -name '*.c')
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
EXE := awale

.PHONY: all clean
all: $(EXE)

# Link
$(EXE): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(EXE) $(OBJECTS)
	@rm -rf $(OBJ_DIR)
