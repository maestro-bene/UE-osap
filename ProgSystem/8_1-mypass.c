/*
 * Auteur(s): Gaël Cudennec
 */

#include <stdio.h>
#include <stdlib.h>
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

// int setsilent(struct termios *initial_term) {
//   int r;
//   struct termios term;
//
//   /* ??????????????????????????????? */
//   return 0;
// }

// int restaure_term(struct termios *initial_term) { ? ? ? ? ? return 0; }

/*  a utiliser au 8.4 */
// char *get_pass() {
//   static char buf[BUFSIZE];
//   char c;
//   int i = 0;
//
//   while ((c = getchar()) != '\n') {
//     buf[i++] = c;
//     putchar('*');
//   }
//
//   buf[i] = '\0';
//   putchar('\n');
//   return buf;
// }

int main() {
  // int r;
  // char buf[BUFSIZE], *s, *pwd;
  // struct termios initial_term;
  char key[3];

  key[0] = getkey();
  key[1] = getkey();
  key[2] = '\0';

  printf("key = %s\n", key);

  // printf("Tapez votre mot de passe : ");
  // setsilent(&initial_term);
  // s = fgets(buf, BUFSIZE, stdin);

  // printf("Mot de passe chiffre : %s\n", pwd);

  // restaure_term(&initial_term);
  exit(EXIT_SUCCESS);
}
