#include <stdio.h>

void usage();

int main(int argc, char **argv) {
    usage();
}

void usage() {
    fprintf(stderr, "usage: tail [file...]\n");
}

