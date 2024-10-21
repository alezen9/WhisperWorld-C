# Makefile to integrate with CMake

build_dir = build

.PHONY: all clean server client

all: init server client

init:
	cmake -B$(build_dir)

server:
	cmake --build $(build_dir) --target server

server-run:
	./$(build_dir)/src/server

server-debug: server
	lldb $(build_dir)/src/server

client:
	cmake --build $(build_dir) --target client

client-run:
	./$(build_dir)/src/client

client-debug: client
	lldb $(build_dir)/src/client

clean:
	@rm -rf $(build_dir)
