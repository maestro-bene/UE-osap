/*
 * Auteur(s):
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void travail() {
  /* Je travaille tres intensement !    */
  /* Ne cherchez pas a comprendre ;-) */
  /* Il n'y a rien a modifier ici     */
  for (;;) {
    printf("%s", random() > RAND_MAX >> 1 ? "." : "\b \b");
    fflush(stdout);
    usleep(100000);
  }
}
void travail() __attribute__((noreturn));
/* Petit raffinement pour le compilateur: cette fonction ne termine pas */

void handle_sigchld(int sig) {
  // Reap terminated child process
  wait(NULL);
}

int main() {
  int pid;

  pid = fork();

  if (pid != 0) { /* Processus Pere */
    signal(SIGCHLD, handle_sigchld);
    travail();
  } else { /* Processus Fils */
    sleep(5);
    printf("Ahrg!!! je suis le fils et je meurs\n");
    exit(EXIT_SUCCESS);
  }
}
