#include <stdlib.h>
#include <stdio.h>
#include "parser.hpp"

char* readfile(char* path, char* mode) {
    FILE* f;

    if(mode == NULL) {
        f = fopen(path, "rb");
    } else {
        f = fopen(path, mode);
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = (char*)malloc(fsize + 1);
    fread(buffer, 1, fsize, f);
    fclose(f);

    buffer[fsize] = 0;
    return buffer;
}

int main(int argc, char** argv) {
    if(argc == 1) {
        fprintf(stderr, "akucc: \033[31;1mfatal\033[0m: no input file specified\n");
        exit(1);
    }
    
    char* input = readfile(argv[1], "rb");
    char* fname = (char*)malloc(128);
    sprintf(fname, "out%s.c", argv[1]);
    Parser parser(input, fname, true, false);
    parser.program();
    parser.emitter.writeFile();
    free(fname);
    return 0;
}