# Project name and directories
PROJECT_NAME = WhisperWorld
COMPILER = gcc
CFLAGS = -Iinclude -Wall   # Include the "include" directory for header files
OUT_DIR = bin
OBJ_DIR = obj

# Source files for server and client
SERVER_SRC = src/server.c src/list.c
CLIENT_SRC = src/client.c src/list.c

# Object files for server and client
SERVER_OBJ = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SERVER_SRC))
CLIENT_OBJ = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(CLIENT_SRC))

# Final binaries
SERVER_BIN = $(OUT_DIR)/$(PROJECT_NAME)-server
CLIENT_BIN = $(OUT_DIR)/$(PROJECT_NAME)-client

# Default target
all: server client

# Compile the server
server: $(SERVER_BIN)

$(SERVER_BIN): $(SERVER_OBJ)
	@mkdir -p $(OUT_DIR)
	$(COMPILER) $(SERVER_OBJ) -o $(SERVER_BIN)

# Compile the client
client: $(CLIENT_BIN)

$(CLIENT_BIN): $(CLIENT_OBJ)
	@mkdir -p $(OUT_DIR)
	$(COMPILER) $(CLIENT_OBJ) -o $(CLIENT_BIN)

# Rule to compile object files from source files
$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(OBJ_DIR)
	$(COMPILER) $(CFLAGS) -c $< -o $@

# Run the server
server-run: server
	@./$(SERVER_BIN)

# Run the client
client-run: client
	@./$(CLIENT_BIN)

# Clean the project
clean:
	rm -rf $(OUT_DIR) $(OBJ_DIR)
