/*
 * Auteur(s): Gaël Cudennec
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#define _XOPEN_SOURCE /* Voir le man 3 crypt */
#include <unistd.h>
/* #include <crypt.h> */

#define BUFSIZE 64
char *c_key =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

char getkey() {
  long nb;
  struct timeval tp;

  gettimeofday(&tp, NULL);
  srandom((unsigned int)tp.tv_usec);
  nb = random() % 64;

  return c_key[nb];
}

int setsilent(struct termios *initial_term) {
  struct termios term;
  tcgetattr(STDIN_FILENO, initial_term); // Sauvegarde la configuration actuelle

  term = *initial_term;
  term.c_lflag &= ~(ECHO | ICANON); // Désactive l'écho et le mode canonique
  term.c_cc[VMIN] = 1;
  term.c_cc[VTIME] = 0;

  tcsetattr(STDIN_FILENO, TCSANOW, &term); // Applique la nouvelle configuration
  return 0;
}

int restaure_term(struct termios *initial_term) {
  tcsetattr(STDIN_FILENO, TCSANOW,
            initial_term); // Restaure la configuration sauvegardée
  return 0;
}

char *get_pass() {
  static char buf[BUFSIZE];
  int i = 0, c;

  printf("Tapez votre mot de passe (ne sera pas affiché): ");
  fflush(stdout); // Assurez-vous que le prompt est affiché immédiatement

  while (i < BUFSIZE - 1 && (c = getchar()) != '\n' && c != EOF) {
    if (c == 127 || c == '\b') { // Gestion de la touche de retour arrière (127
      // est le code ASCII DEL)
      if (i > 0) {
        i--;
        printf("\b \b"); // Efface le dernier '*' affiché
        fflush(stdout);
      }
    } else {
      buf[i++] = (char)c;
      printf("*");
      fflush(stdout);
    }
  }

  buf[i] = '\0';
  printf("\n"); // Nouvelle ligne après la fin de la saisie du mot de passe
  return buf;
}

int main() {
  // int r;
  char *s, *pwd;
  struct termios initial_term;
  char key[3];

  key[0] = getkey();
  key[1] = getkey();
  key[2] = '\0';

  printf("Tapez votre mot de passe : ");
  // Lire le mot de passe
  setsilent(&initial_term);
  s = get_pass();

  // Chiffrer le mot de passe
  pwd = crypt(s, key);

  printf("key = %s\n", key);
  printf("Mot de passe chiffre : %s\n", pwd);

  restaure_term(&initial_term);
  exit(EXIT_SUCCESS);
}
