#! /bin/bash

echo "testing basic.aku..."
echo ""

time (
    echo "compiling libaku..."
    cd stdlib && 
    make && 
    cd .. &&
    echo "" &&
    echo "transpiling basic.aku to C..." &&
    ./bin/akucc basic.aku &&
    echo "done." &&
    echo "compiling C output..." &&
    gcc -o basic outbasic.aku.c stdlib/build/libaku.a -Istdlib/declarations &&
    rm outbasic.aku.c &&
    echo "done." &&
    echo "output:" &&
    ./basic &&
    rm basic &&
    echo "" &&
    echo "finished execution." &&
    echo "" &&
    echo "done in:"
)