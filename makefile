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
run:
	@$(patsubst %,$(OUT_DIR)/%,$(NAME)).o
clean:
	@rm -f $(OUT_DIR) &&
debug: compile run
