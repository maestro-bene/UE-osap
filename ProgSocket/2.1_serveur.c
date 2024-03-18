#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFSIZE 513 // Adjusted to allow space for the null terminator

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <socket_path>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int sfd, ns;
  struct sockaddr_un sun;
  char buf[BUFSIZE];
  ssize_t nread, nwrite;
  struct sockaddr_storage from;
  socklen_t fromlen;
  char *message = "Message to send: ";

  if ((sfd = socket(PF_LOCAL, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  memset(&sun, 0, sizeof(struct sockaddr_un)); // Zero out the structure
  sun.sun_family = AF_UNIX;
  strncpy(sun.sun_path, argv[1],
          sizeof(sun.sun_path) - 1); // Ensure null-termination

  if (bind(sfd, (struct sockaddr *)&sun, sizeof(sun)) == -1) {
    perror("bind");
    close(sfd);
    exit(EXIT_FAILURE);
  }

  if (listen(sfd, SOMAXCONN) == -1) {
    perror("listen");
    close(sfd);
    exit(EXIT_FAILURE);
  }

  printf("Server is listening on Unix domain socket at %s...\n", argv[1]);

  for (;;) {
    fromlen = sizeof(from);
    ns = accept(sfd, (struct sockaddr *)&from, &fromlen);
    if (ns == -1) {
      perror("accept");
      continue;
    }

    printf("Connected to a client.\n");

    nwrite = write(ns, message, strlen(message));
    if (nwrite == -1) {
      perror("write");
      close(ns);
      continue;
    }

    while ((nread = read(ns, buf, BUFSIZE - 1)) >
           0) { // Adjust buffer size for null terminator
      buf[nread] = '\0';
      printf("Message received: %s\n", buf);
      nwrite = write(ns, message, strlen(message));
      if (nwrite == -1) {
        perror("write");
        close(ns);
        continue;
      }
    }

    if (nread == -1) {
      perror("read");
    }

    close(ns);
    printf("Connection closed.\n");
  }

  close(sfd);
  unlink(argv[1]); // Use the provided socket path for cleanup

  return 0;
}
