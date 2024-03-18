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

    switch (pid) {
    case 0:
      wait(NULL);
      break;
    default: {
      // Here we use execlp which looks in PATH to find the executable given as
      // first argument
      int exer = execlp(s, ".");
      if (exer) {
        fprintf(stderr, "Erreur dans le exec\n");
        perror("Error");
        // Exit the child process, this is what was missing
        exit(pid);
      }
    }
    }
  }
}
