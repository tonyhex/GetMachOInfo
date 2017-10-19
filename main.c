#include <stdio.h>
#include "macho_reader.h"

int main(int argc, char *argv[]) {
    const char *filename = argv[1];
    FILE *file_handle = fopen(filename, "rb");
    print_mach_info(file_handle);
    return 0;
}