CC = gcc
CFLAGS_DEBUG = -g -ggdb -pthread -std=c17 -pedantic -W -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
CFLAGS_RELEASE = -std=c17 -pthread -pedantic -W -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable

SRC_DIR = src
BUILD_DIR = build
DEBUG_DIR = $(BUILD_DIR)/debug
RELEASE_DIR = $(BUILD_DIR)/release

SRC_FILES = $(SRC_DIR)/main.c $(SRC_DIR)/func.c
OBJ_FILES_DEBUG = $(patsubst $(SRC_DIR)/%.c, $(DEBUG_DIR)/%.o, $(SRC_FILES))
OBJ_FILES_RELEASE = $(patsubst $(SRC_DIR)/%.c, $(RELEASE_DIR)/%.o, $(SRC_FILES))
EXEC_NAME = prog

all: debug

debug: $(DEBUG_DIR)/$(EXEC_NAME)

release: $(RELEASE_DIR)/$(EXEC_NAME)

$(DEBUG_DIR)/$(EXEC_NAME): $(OBJ_FILES_DEBUG) | $(DEBUG_DIR)
	$(CC) -pthread $^ -o $@

$(RELEASE_DIR)/$(EXEC_NAME): $(OBJ_FILES_RELEASE) | $(RELEASE_DIR)
	$(CC) -pthread $^ -o $@

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.c | $(DEBUG_DIR)
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.c | $(RELEASE_DIR)
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@

$(DEBUG_DIR) $(RELEASE_DIR):
	@mkdir -p $@

clean:
	rm -fr $(BUILD_DIR)

.PHONY: all debug release clean