RED=$(shell tput -Txterm setaf 1)
GREEN=$(shell tput -Txterm setaf 2)
RESET=$(shell tput sgr0)

COMPILER=gcc
OUT_DIR=obj

NAME = chat-app
OUT = $(patsubst %,$(OUT_DIR)/%,$(NAME).o)

compile:
	@mkdir -p $(OUT_DIR)
	@$(COMPILER) -o $(OUT) main.c
	@echo "[Chat application] $(GREEN)Compiled successfully$(RESET)"
run:
	@$(patsubst %,$(OUT_DIR)/%,$(NAME)).o
clean:
	@rm -r $(OUT_DIR) &&
	@echo "[Chat application] $(GREEN)Cleaned up successfully$(RESET)"
debug: compile run
