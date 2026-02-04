#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("Assignment 1 by Kyle Parker\n");
    printf("There are a total of %d arguments for this program\n", argc);
    printf("The command that was run is %s\n", argv[0]);

    if (argc == 1) {
        printf("No command line arguments were provided to this program\n");
    } else {
        printf("The command line arguments for the program are:\n");
        for (int i = 1; i < argc; i++) {
            printf("Argument %d: %s\n", i, argv[i]);
        }
    }

    return 0;
}
