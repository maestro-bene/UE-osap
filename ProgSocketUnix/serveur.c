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

#define SOCKET_ADDR "/tmp/android-maestro-bene"
#define BUFSIZE 512

int main(int argc, char **argv) {
  int sfd, ns;
  struct sockaddr_un sun;
  char buf[BUFSIZE];
  ssize_t nread, nwrite;
  struct sockaddr_storage from;
  socklen_t fromlen;
  char *message = "Message to send: ";

  if ((sfd = socket(AF_UNIX, SOCK_SEQPACKET, 0)) == 0) {
    fprintf(stderr, "ERROR opening socket");
    exit(1);
  }
  sun.sun_family = AF_UNIX;
  strlcpy(sun.sun_path, SOCKET_ADDR, sizeof(sun.sun_path));

  if (bind(sfd, (struct sockaddr *)&sun, sizeof(sun)) == 0) {
    close(sfd);
    perror("bind");
  }

  // Listen for connections on the socket
  if (listen(sfd, SOMAXCONN) == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  printf("Server is listening on Unix domain socket...\n");

  // Accept connections from clients
  for (;;) {
    fromlen = sizeof(from);
    ns = accept(sfd, (struct sockaddr *)&from, &fromlen);
    if (ns == -1) {
      perror("accept");
      continue;
    }

    printf("Connected to a client.\n");

    // Send a message to the client
    nwrite = write(ns, message, strlen(message));
    if (nwrite == -1) {
      perror("write");
      close(ns);
      continue;
    }

    // Receive messages from the client
    while ((nread = read(ns, buf, BUFSIZE)) > 0) {
      buf[nread] = '\0';
      printf("Message received: %s\n", buf);
    }

    if (nread == -1) {
      perror("read");
    }

    // Close the connection
    close(ns);
    printf("Connection closed.\n");
  }

  // Close the socket
  close(sfd);

  // Unlink the socket path
  unlink(SOCKET_ADDR);

  return 0;
}
