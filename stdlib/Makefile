CC = gcc
AR = ar
AKUCC = ../bin/akucc

all: io os package

io:
	@echo "compiling \`io\`..."
	@$(AKUCC) io.aku
	@mkdir -p build
	@$(CC) -c outio.aku.c -o build/io.o
	@rm outio.aku.c
	@echo "done."

os:
	@echo "compiling \`os\`..."
	@mkdir -p build
	@$(AKUCC) os.aku
	@$(CC) -c outos.aku.c -o build/os.o
	@rm outos.aku.c
	@echo "done."

package:
	@echo "packaging \`libaku\` with \`$(AR)\`..."
	@$(AR) rs build/libaku.a build/*
	@echo "done."