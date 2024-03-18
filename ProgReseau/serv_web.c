#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFSIZE 512
enum TypeFichier { NORMAL, REPERTOIRE, ERREUR };

const char *OK200 = "HTTP/1.1 200 OK\r\nConnection: close\r\n";

const char *ERROR403 =
    "HTTP/1.1 403 Forbidden\r\nConnection: close\r\n\r\nAccess denied\r\n";

const char *ERROR404 = "HTTP/1.1 404 Not Found\r\nConnection: "
                       "close\r\n\r\nFile or directory not found\r\n";

void sendContentType(int soc, const char *filePath) {
  const char *extension = strrchr(filePath, '.');
  const char *header;

  if (extension) {
    if (strcmp(extension, ".html") == 0) {
      header = "Content-Type: text/html; charset=UTF-8\r\n\r\n";
    } else if (strcmp(extension, ".ico") == 0) {
      header = "Content-Type: image/x-icon\r\n\r\n";
    } else if (strcmp(extension, ".gif") == 0) {
      header = "Content-Type: image/gif\r\n\r\n";
    } else {
      header = "Content-Type: application/octet-stream\r\n\r\n";
    }
  } else {
    header = "Content-Type: application/octet-stream\r\n\r\n";
  }
  write(soc, header, strlen(header));
}

/* Fonction typeFichier()
 * argument: le nom du fichier
 * rend une valeur de type enumeration delaree en tete du fichier
 * NORMAL, ou REPERTOIRE ou ERRREUR
 */
enum TypeFichier typeFichier(char *fichier) {
  struct stat status;
  int r;

  r = stat(fichier, &status);
  if (r < 0)
    return ERREUR;
  if (S_ISREG(status.st_mode))
    return NORMAL;
  if (S_ISDIR(status.st_mode))
    return REPERTOIRE;
  /* si autre type, on envoie ERREUR (a fixer plus tard) */
  return ERREUR;
}

/* envoiFichier()
 * Arguments: le nom du fichier, la socket
 * valeur renvoyee: true si OK, false si erreur
 */
bool envoiFichier(char *fichier, int soc) {
  if (access(fichier, R_OK) != 0) {
    write(soc, ERROR403, strlen(ERROR403));
    return false;
  }

  int fd = open(fichier, O_RDONLY);
  if (fd < 0) {
    write(soc, ERROR404, strlen(ERROR404));
    return false;
  }

  // Move content type sending here
  write(soc, OK200, strlen(OK200)); // Now we're sure the file is accessible
  sendContentType(soc, fichier);    // Set the correct content type header

  char buf[BUFSIZE];
  ssize_t nread;
  while ((nread = read(fd, buf, BUFSIZE)) > 0) {
    if (write(soc, buf, nread) != nread) {
      perror("write");
      close(fd);
      return false;
    }
  }
  close(fd);
  return true;
}

/* envoiRep()
 * Arguments: le nom du repertoire, la socket
 * valeur renvoyee: true si OK, false si erreur
 */
bool envoiRep(char *rep, int soc) {
  DIR *dp = opendir(rep);
  if (dp == NULL) {
    write(soc, ERROR404, strlen(ERROR404));
    return false;
  }

  write(soc, OK200, strlen(OK200));
  sendContentType(
      soc, "/index.html"); // Set the correct content type header forcefully
  struct dirent *entry;
  char buf[1024];
  while ((entry = readdir(dp)) != NULL) {
    // Determine the full path to the entry
    char fullPath[1024];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", rep, entry->d_name);

    // Determine the icon based on the type
    const char *icon;
    if (typeFichier(fullPath) == REPERTOIRE) {
      icon = "icons/folder.gif";
    } else {
      icon = "icons/generic.gif";
    }

    // Generate the HTML snippet with the icon
    snprintf(
        buf, sizeof(buf),
        "<div><img src=\"%s\" style=\"width:16px;height:16px;\"> %s</div>\n",
        icon, entry->d_name);
    write(soc, buf, strlen(buf));
  }
  closedir(dp);
  return true;
}

void communication(int soc, struct sockaddr *from, socklen_t fromlen) {
  char buf[BUFSIZE], method[10], path[BUFSIZE], protocol[10];
  ssize_t nread;
  char host[NI_MAXHOST];

  /* Reconnaissance de la machine appelante */
  int s = getnameinfo(from, fromlen, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
  if (s == 0) {
    printf("Connection from %s\n", host);
  } else {
    fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
  }

  /* Lecture de la requête HTTP */
  nread = read(soc, buf, BUFSIZE - 1);
  if (nread <= 0) {
    perror("Erreur lecture socket");
    return;
  }
  buf[nread] = '\0'; // Assure la présence du terminateur de chaîne

  /* Décodage de la requête HTTP */
  // Exemple de requête : GET /index.html HTTP/1.1
  if (sscanf(buf, "%s %s %s", method, path, protocol) != 3) {
    perror("Erreur format requête");
    return;
  }

  printf("Method: %s, Path: %s, Protocol: %s\n", method, path, protocol);

  /* Gestion des requêtes GET uniquement */
  if (strcmp(method, "GET") == 0) {
    // Enlève le slash du début du chemin
    char *filePath = path;

    if (strcmp(path, "/") == 0) {
      filePath = "."; // Translate "/" to "." for filesystem operations
    } else {
      filePath = path + 1; // Remove the leading slash for other paths
    }

    enum TypeFichier fileType = typeFichier(filePath);

    switch (fileType) {
    case NORMAL:
      printf("FileType: NORMAL\n");
      envoiFichier(filePath, soc);
      break;
    case REPERTOIRE:
      printf("FileType: REPERTOIRE\n");
      envoiRep(filePath, soc);
      break;
    case ERREUR:
    default:
      printf("FileType: ERROR\n");
      write(soc, ERROR404, strlen(ERROR404));
      break;
    }
  } else {
    // Réponse pour méthodes non gérées
    write(soc, "HTTP/1.1 501 Not Implemented\r\nConnection: close\r\n\r\n",
          strlen("HTTP/1.1 501 Not Implemented\r\nConnection: close\r\n\r\n"));
  }

  close(soc);
}

int main(int argc, char **argv) {
  int sfd, s, ns, r, pid;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  struct sockaddr_storage from;
  socklen_t fromlen;

  if (argc != 2) {
    printf("Usage: %s  port_serveur\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Inserer ici le code d'un serveur TCP concurent */
  char host[NI_MAXHOST];

  /* Construction de l'adresse locale (pour bind) */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET6;             /* Force IPv6 */
  hints.ai_socktype = SOCK_STREAM;        /* Stream socket */
  hints.ai_flags = AI_PASSIVE;            /* Adresse IP joker */
  hints.ai_flags |= AI_V4MAPPED | AI_ALL; /* IPv4 remapped en IPv6 */
  hints.ai_protocol = 0;                  /* Any protocol */

  s = getaddrinfo(NULL, argv[1], &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  /* getaddrinfo() retourne une liste de structures d'adresses.
     On essaie chaque adresse jusqu'a ce que bind(2) reussisse.
     Si socket(2) (ou bind(2)) echoue, on (ferme la socket et on)
     essaie l'adresse suivante. cf man getaddrinfo(3) */
  for (rp = result; rp != NULL; rp = rp->ai_next) {

    /* Creation de la socket */
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

    if (sfd == -1)
      continue;

    /* Association d'un port a la socket */
    r = bind(sfd, rp->ai_addr, rp->ai_addrlen);

    if (r == 0)
      break; /* Succes */
    close(sfd);
  }

  if (rp == NULL) { /* Aucune adresse valide */
    perror("bind");
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(result); /* Plus besoin */

  /* Positionnement de la machine a etats TCP sur listen */
  listen(sfd, BUFSIZE);

  for (;;) {
    /* Acceptation de connexions */
    fromlen = sizeof(from);

    ns = accept(sfd, (struct sockaddr *)&from, &fromlen);

    if (ns == -1) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    pid = fork();

    switch (pid) {
    case -1:
      perror("fork");
    // Processus fils
    case 0:
      /* Reconnaissance de la machine cliente */
      printf("Debut du processus fils : ");
      printf("%d \n", getpid());
      s = getnameinfo((struct sockaddr *)&from, fromlen, host, NI_MAXHOST, NULL,
                      0, NI_NUMERICHOST);
      if (s == 0) {
        printf("Debut avec client '%s'\n", host);
        communication(ns, (struct sockaddr *)&from, fromlen);
      } else
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
      printf("Terminaison du processus fils\n");
      exit(0);
    // Processus pere
    default:
      signal(SIGCHLD, SIG_IGN);
      printf("Continuation du processus pere : ");
      printf("%d \n", getpid());
      break;
    }
  }
}
