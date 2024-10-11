# Colors for output
RED=$(shell tput -Txterm setaf 1)
GREEN=$(shell tput -Txterm setaf 2)
RESET=$(shell tput sgr0)

# Compiler and flags
COMPILER = gcc
CFLAGS = -Wall -Werror

# Output directory
OUT_DIR = obj

# Program name
NAME = WisperWorld
OUT = $(OUT_DIR)/$(NAME)

# Source and object files
SRC = main.c list.c
OBJ = $(patsubst %.c,$(OUT_DIR)/%.o,$(SRC))

# Default target: Build and link the program
all: $(OUT)
	@echo "$(GREEN)Build complete!$(RESET)"

# Compile the program by linking object files
$(OUT): $(OBJ)
	@mkdir -p $(OUT_DIR)
	@$(COMPILER) $(CFLAGS) -o $(OUT) $(OBJ)
	@echo "$(GREEN)Linked $(OUT)!$(RESET)"

# Compile object files from source files
$(OUT_DIR)/%.o: %.c
	@mkdir -p $(OUT_DIR)
	@$(COMPILER) $(CFLAGS) -c $< -o $@
	@echo "$(GREEN)Compiled $<$(RESET)"

# Run the program
run: $(OUT)
	@./$(OUT)

# Clean up the build (remove object files and binary)
clean:
	@rm -rf $(OUT_DIR)
	@echo "$(RED)Build cleaned!$(RESET)"

# Debug build (adds debugging symbols)
debug: CFLAGS += -g
debug: clean all
	@echo "$(GREEN)Debug build complete!$(RESET)"
