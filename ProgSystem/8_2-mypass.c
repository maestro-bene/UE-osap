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

char *get_pass() {
  static char buf[BUFSIZE];
  printf("Tapez votre mot de passe (sera affiché): ");
  fgets(buf, BUFSIZE, stdin);

  // Remplacer le saut de ligne par un caractère nul '\0'
  if (buf[strlen(buf) - 1] == '\n') {
    buf[strlen(buf) - 1] = '\0';
  }

  return buf;
}

int main() {
  // int r;
  char *s, *pwd;
  // struct termios initial_term;
  char key[3];

  key[0] = getkey();
  key[1] = getkey();
  key[2] = '\0';

  printf("Tapez votre mot de passe : ");
  // setsilent(&initial_term);
  // Lire le mot de passe
  s = get_pass();

  // Chiffrer le mot de passe
  pwd = crypt(s, key);

  printf("key = %s\n", key);
  printf("Mot de passe chiffre : %s\n", pwd);

  // restaure_term(&initial_term);
  exit(EXIT_SUCCESS);
}
