# Compilateur et options
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -O2 -g
LDFLAGS :=
RM := rm -f

# Répertoires
SRC_DIR := src
OBJ_DIR := build
BIN_DIR := bin

# Fichiers par composant
JEU_SRCS := $(shell find $(SRC_DIR)/jeu -name '*.c')
JEU_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(JEU_SRCS))
JEU_BIN := $(BIN_DIR)/awale

SERVER_SRCS := $(shell find $(SRC_DIR)/serveur -name '*.c')
SERVER_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SERVER_SRCS))
SERVER_BIN := $(BIN_DIR)/serveur

CLIENT_SRCS := $(shell find $(SRC_DIR)/client -name '*.c')
CLIENT_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CLIENT_SRCS))
CLIENT_BIN := $(BIN_DIR)/client

# Règles de construction
.PHONY: all clean install
all: $(JEU_BIN) $(SERVER_BIN) $(CLIENT_BIN)

# Edition des liens (liaison)
$(JEU_BIN): $(JEU_OBJS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)

$(SERVER_BIN): $(SERVER_OBJS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)

$(CLIENT_BIN): $(CLIENT_OBJS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(SERVER_BIN) $(CLIENT_BIN) $(OBJECTS)
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
