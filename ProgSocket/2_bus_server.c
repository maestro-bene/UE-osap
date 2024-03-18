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

int server;     /* socket du serveur en attente de connection */
char *sockname; /* nom de la socket */

static inline int max(int x, int y) { return x > y ? x : y; }

void hdlr_fin(int sig) {
  printf("Reception du signal %d. Arret du bus !\n", sig);
  close(server);
  unlink(sockname);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  struct sockaddr_un a;
  int *clients;    /* sockets des clients */
  int max_clients; /* nombre de clients */
  char buffer[BUF_SIZE];
  // int r, i, j;  // Commented initialization of r as not directly used
  int i, j;
  ssize_t rd_sz, wr_sz;

  if (argc != 3) {
    fprintf(stderr, "Utilisation:\n\t%s <Unix socket path> <max clients>\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }

  signal(SIGPIPE, SIG_IGN);

  signal(SIGQUIT, hdlr_fin);
  signal(SIGTERM, hdlr_fin);
  signal(SIGINT, hdlr_fin);
  signal(SIGSEGV, hdlr_fin);

  max_clients = atoi(argv[2]);
  clients = (int *)malloc(max_clients * sizeof(int));
  for (i = 0; i < max_clients; i++)
    clients[i] = -1;

  if ((server = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("socket()");
    exit(EXIT_FAILURE);
  }

  memset(&a, 0, sizeof(a));
  a.sun_family = AF_UNIX;
  sockname = strncpy(a.sun_path, argv[1], sizeof(a.sun_path) - 1);

  if (bind(server, (struct sockaddr *)&a, sizeof(a)) < 0) {
    perror("bind()");
    close(server);
    exit(EXIT_FAILURE);
  }
  if (listen(server, SOMAXCONN) < 0) {
    perror("listen()");
    close(server);
    exit(EXIT_FAILURE);
  }

  printf("Demarrage du bus.\n");
  for (;;) {
    int nfds = 0;
    fd_set rd_set;

    FD_ZERO(&rd_set);
    FD_SET(server, &rd_set);
    nfds = max(nfds, server);

    for (i = 0; i < max_clients; i++) {
      if (clients[i] > 0) {
        FD_SET(clients[i], &rd_set);
        nfds = max(nfds, clients[i]);
      }
    }

    if (select(nfds + 1, &rd_set, NULL, NULL, NULL) < 0) {
      if (errno == EINTR)
        continue;
      perror("select()");
      exit(EXIT_FAILURE);
    }

    if (FD_ISSET(server, &rd_set)) {
      int new_client = accept(server, NULL, NULL);
      if (new_client < 0) {
        perror("accept()");
      } else {
        for (i = 0; (i < max_clients) && (clients[i] > 0); i++)
          ;
        if (i < max_clients) {
          printf("Un nouveau client %d se connecte.\n", i);
          clients[i] = new_client;
        } else {
          printf("Plus de place pour un nouveau client.\n");
          close(new_client);
        }
      }
    }

    for (i = 0; i < max_clients; i++) {
      if ((clients[i] > 0) && FD_ISSET(clients[i], &rd_set)) {
        rd_sz = recv(clients[i], buffer, BUF_SIZE, 0);
        if (rd_sz < 0) {
          perror("recv()");
          fprintf(stderr, "...probleme avec le client %d\n", i);
          shutdown(clients[i], SHUT_RDWR);
          clients[i] = -1;
        } else if (rd_sz == 0) {
          printf("Le client %d et partit.\n", i);
          close(clients[i]);
          clients[i] = -1;
        } else {
          printf("Reception de %zd octets du client %d : [\n%s]\n", rd_sz, i,
                 buffer);
          for (j = 0; j < max_clients; j++) {
            if ((clients[j] > 0) && (i != j)) {
              wr_sz = send(clients[j], buffer, rd_sz, 0);
              if (wr_sz < 0) {
                perror("send()");
                fprintf(stderr, "...probleme avec le client %d\n", j);
                close(clients[j]);
                clients[j] = -1;
              } else {
                printf("Envoie de %zd octets au client %d.\n", wr_sz, j);
              }
            }
          }
        }
      }
    }
  }
  exit(EXIT_SUCCESS);
}
