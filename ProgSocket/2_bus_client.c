#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define BUF_SIZE 1024

int server; /* socket du serveur en attente de connection */

static inline int max(int x, int y) { return x > y ? x : y; }

void hdlr_fin(int sig) {
  printf("Reception du signal %d. Arret du client !\n", sig);
  close(server);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  struct sockaddr_un a;
  char buffer[BUF_SIZE];
  // int r;  // Commented out as not used in the completed snippets
  ssize_t rd_sz, wr_sz;

  if (argc != 2) {
    fprintf(stderr, "Utilisation:\n\t%s <Unix socket path>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  signal(SIGPIPE, SIG_IGN);

  /* pour une terminaison propre sur QUIT TERM INT SEGV... */
  signal(SIGQUIT, hdlr_fin);
  signal(SIGTERM, hdlr_fin);
  signal(SIGINT, hdlr_fin);
  signal(SIGSEGV, hdlr_fin);

  /* creation de la socket client */
  if ((server = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("socket()");
    exit(EXIT_FAILURE);
  }

  memset(&a, 0, sizeof(a));
  a.sun_family = AF_UNIX;
  strncpy(a.sun_path, argv[1], sizeof(a.sun_path) - 1);

  if (connect(server, (struct sockaddr *)&a, sizeof(a)) < 0) {
    perror("connect()");
    close(server);
    exit(EXIT_FAILURE);
  }

  printf("Le client est accroche au bus.\n");
  for (;;) {
    int nfds = 0;
    fd_set rd_set;

    FD_ZERO(&rd_set);

    FD_SET(server, &rd_set);
    nfds = max(nfds, server);

    FD_SET(STDIN_FILENO, &rd_set);
    nfds = max(nfds, STDIN_FILENO);

    if (select(nfds + 1, &rd_set, NULL, NULL, NULL) < 0) {
      if (errno == EINTR)
        continue;
      perror("select()");
      exit(EXIT_FAILURE);
    }

    if (FD_ISSET(server, &rd_set)) {
      rd_sz = recv(server, buffer, BUF_SIZE, 0);
      if (rd_sz < 0) {
        perror("recv()");
        close(server);
        exit(EXIT_FAILURE);
      } else if (rd_sz == 0) {
        printf("Le bus est arrete !\n");
        exit(EXIT_SUCCESS);
      } else {
        printf("Reception de %zd octets : [\n%s]\n", rd_sz, buffer);
      }
    }

    if (FD_ISSET(STDIN_FILENO, &rd_set)) {
      fgets(buffer, BUF_SIZE, stdin);
      rd_sz = strlen(buffer);
      wr_sz = send(server, buffer, rd_sz, 0);
      if (wr_sz < 0) {
        perror("send()");
        close(server);
        exit(EXIT_FAILURE);
      } else {
        printf("Envoie de %zd octets\n", wr_sz);
      }
    }
  }
  exit(EXIT_SUCCESS);
}
