#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <hex_address>\n", argv[0]);
        return 1;
    }

    // Convert hex string to unsigned long
    unsigned long logical_addr = strtoul(argv[1], NULL, 16);

    // Page size = 4KB = 2^12
    unsigned long offset = logical_addr & 0xFFF;
    unsigned long page_index = logical_addr >> 12;

    printf("Logical Addr:0x%08lX - Page Index:0x%08lX - Offset:0x%08lX\n",
           logical_addr, page_index, offset);

    return 0;
}