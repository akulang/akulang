CC = gcc
AR = ar
AKUCC = ../bin/akucc

all: io package

io:
	@echo "compiling \`io\`..."
	@$(AKUCC) io.aku
	@$(CC) -c outio.aku.c -o build/io.o
	@rm outio.aku.c
	@echo "done."

package:
	@echo "packaging \`libaku\` with \`$(AR)\`..."
	@$(AR) rs build/libaku.a build/*
	@echo "done."