/*
 * Auteur(s): Cudennec Gaël
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define TABSIZE 512

int main() {
    int pid, longueur;
    char tab[TABSIZE], *s;

    for (;;) {
        fputs("petit_shell...> ", stdout); /* Affichage d'un prompt */
        s = fgets(tab, TABSIZE, stdin);

        if (s == NULL) {
            fprintf(stderr, "Fin du Shell\n");
            exit(EXIT_SUCCESS);
        }

        longueur = strlen(s);
        tab[longueur - 1] = '\0';

        pid = fork();

        if (pid == 0) {
            wait(NULL);
        } else {
            int exer = execl(s, ".");
            if (exer) {
                fprintf(stderr, "Erreur dans le exec\n");
                perror("Error");
            }
        }
    }
}
