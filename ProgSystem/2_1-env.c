/*
 * Auteur(s): Cudennec Gaël
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Environ is a external variable, and returns pointer to an array of strings
    extern char **environ;
    int i = 0;
    // Loop until null pointer (end of the environ)
    while (environ[i] != NULL) {
        printf("%s \n", environ[i]);
        i++;
    }
    return EXIT_SUCCESS;
}
