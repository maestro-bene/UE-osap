#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE 100000

int main(int argc, char **argv) {
  int sfd, s, rsz;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  ssize_t nrecv;
  char buf[BUFSIZE];
  struct sockaddr_storage from;
  socklen_t fromlen;
  char host[NI_MAXHOST], service[NI_MAXSERV];

  if (argc != 3) { // Adjusted to accept both port and multicast IP as arguments
    printf("Usage: %s multicast_ip multicast_port\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Construction de l'adresse locale (pour bind) */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC; // Autorise IPv4 ou IPv6
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // Utilise l'adresse IP de l'hôte

  s = getaddrinfo(argv[1], argv[2], &hints, &result); // Use the provided port
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  // Iterate through the address list and try to create and bind a socket
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1)
      continue;

    if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
      break; // Success

    close(sfd);
  }

  if (rp == NULL) { // No address succeeded
    fprintf(stderr, "Could not bind\n");
    exit(EXIT_FAILURE);
  }

  // Join multicast group
  if (rp->ai_family == AF_INET) { // IPv4
    struct ip_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(argv[1]); // Use the provided IP
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) <
        0) {
      perror("setsockopt(IP_ADD_MEMBERSHIP) failed");
      exit(EXIT_FAILURE);
    }
  } else if (rp->ai_family == AF_INET6) { // IPv6
    struct ipv6_mreq mreq6;
    memset(&mreq6, 0, sizeof(struct ipv6_mreq));
    if (inet_pton(AF_INET6, argv[1], &mreq6.ipv6mr_multiaddr) != 1) {
      perror("inet_pton() failed");
      exit(EXIT_FAILURE);
    }
    mreq6.ipv6mr_interface = 0;
    if (setsockopt(sfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6)) <
        0) {
      perror("setsockopt(IPV6_JOIN_GROUP) failed");
      exit(EXIT_FAILURE);
    }
  }

  freeaddrinfo(result); // No longer needed

  /* Force la taille du buffer de reception de la socket */
  rsz = 800000;
  if (setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &rsz, sizeof(rsz)) == 0)
    printf("SO_RCVBUF apres forcage: %d octets\n", rsz);
  else
    perror("setsockopt SO_RCVBUF");

  /* Boucle de communication */
  for (;;) {
    /* Reception donnees */
    fromlen = sizeof(from);
    nrecv = recvfrom(sfd, buf, BUFSIZE, 0, (struct sockaddr *)&from, &fromlen);
    if (nrecv == -1) {
      perror("Erreur en lecture socket\n");
      exit(EXIT_FAILURE);
    }
    printf("Recu %zd octets\n", nrecv);

    /* Reconnaissance de la machine cliente */
    s = getnameinfo((struct sockaddr *)&from, fromlen, host, NI_MAXHOST,
                    service, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
    if (s == 0)
      printf("Emetteur '%s'  Port '%s'\n", host, service);
    else
      printf("Erreur: %s\n", gai_strerror(s));
  }
}
