#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <assembly_file>\n", argv[0]);
    }
    return 0;
}
