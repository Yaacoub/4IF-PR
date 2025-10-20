# Makefile multi-target: server et client (corregido)
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -O2 -g -I./src
LDLIBS :=

SRC_DIR := src
BIN_DIR := bin
BUILD_DIR := build

# fuentes por target
SRV_SRCS := $(wildcard $(SRC_DIR)/server/*.c) $(wildcard $(SRC_DIR)/jeu/*.c)
CLT_SRCS := $(wildcard $(SRC_DIR)/client/*.c)

# objetos mantienen la ruta original dentro de build/
SRV_OBJS := $(SRV_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
CLT_OBJS := $(CLT_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean

all: $(BIN_DIR)/server $(BIN_DIR)/client

# Binaries
$(BIN_DIR)/server: $(SRV_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRV_OBJS) $(LDLIBS)
	@echo "Compilé: $@"

$(BIN_DIR)/client: $(CLT_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(CLT_OBJS) $(LDLIBS)
	@echo "Compilé: $@"

# Regla genérica: build/src/.../file.o depende de src/.../file.c
# nota: $(@:$(BUILD_DIR)/%=%) transforma build/src/..../f.o -> src/..../f.o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Crear bin dir
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Nettoyé"

