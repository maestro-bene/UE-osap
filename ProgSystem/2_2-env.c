/*
 * Auteur(s): Cudennec Gaël
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // getenv() returns a pointer to a string
    char *env = getenv(argv[1]);
    printf("%s \n", env);
    return EXIT_SUCCESS;
}
