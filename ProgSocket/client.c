#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BUFSIZE 512

int main(int argc, char **argv) {
  int sfd;
  struct sockaddr_un sun;
  char buf[BUFSIZE], *pt;
  ssize_t nread, nwrite;

  if (argc != 2) {
    printf("Usage: %s chemin_socket\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Create a Unix domain socket
  if ((sfd = socket(AF_UNIX, SOCK_SEQPACKET, 0)) == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // Set up the server address structure
  sun.sun_family = AF_UNIX;
  strcpy(sun.sun_path, argv[1]);

  // Connect to the server
  if (connect(sfd, (struct sockaddr *)&sun, sizeof(sun)) == -1) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  printf("Connected to the server.\n");

  // Communication loop
  for (;;) {
    // Read from the socket
    nread = read(sfd, buf, BUFSIZE);
    if (nread == 0) {
      printf("Connection closed by server.\n");
      break;
    } else if (nread < 0) {
      perror("read");
      exit(EXIT_FAILURE);
    }
    buf[nread] = '\0';
    printf("Message received: '%s'\n", buf);

    // Read from stdin
    printf("Enter message: ");
    pt = fgets(buf, BUFSIZE, stdin);
    if (pt == NULL) {
      printf("End of input. Exiting.\n");
      break;
    }

    // Send message to server
    nwrite = write(sfd, buf, strlen(buf));
    if (nwrite < 0) {
      perror("write");
      exit(EXIT_FAILURE);
    }
  }

  // Close the socket
  close(sfd);

  return 0;
}
