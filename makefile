# Compilateur et options
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -O2 -g
LDFLAGS :=
RM := rm -f

# Répertoires
SRC_DIR := src
OBJ_DIR := build

# Fichiers
SOURCES := $(shell find $(SRC_DIR) -name '*.c')
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
EXE := awale

# Règles de construction
.PHONY: all clean
all: $(EXE)

# Edition des liens (liaison)
$(EXE): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(EXE) $(OBJECTS)
	@rm -rf $(OBJ_DIR)
