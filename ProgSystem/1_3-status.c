/*
 * Auteur(s): Cudennec Gaël
 *
 *
 * Ce programme refait ce que fait la commande "ls". Il donne des
 * informnations sur les caracteristiques de fichiers dont le nom est passe
 * en parametre.
 *
 * Utilisation de la primitive stat(2) et de la fonction getpwuid(3).
 */

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Petite fonction qui se charge d'envoyer les messages d'erreur
   et qui ensuite "suicide" le processus. */

void erreur_grave(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

/* Fonction principale (fournie avec erreur(s?)) */

int main(int argc, char **argv) {
    struct stat status, *buffer;
    buffer = malloc(sizeof(struct stat));
    int r;

    r = stat(argv[1], buffer);
    if (r < 0) {
        erreur_grave("Stat");
    }

    struct passwd *result = getpwuid(buffer->st_uid);
    if (result == NULL) {
        erreur_grave("Stat");
    }

    uint false_uid = 1000;
    struct passwd *result_false = getpwuid(false_uid);
    if (result == NULL) {
        erreur_grave("Stat");
    }

    printf("Fichier %s:  mode: %X  Taille: %lld  Proprietaire: %s\n", argv[1],
           buffer->st_mode, buffer->st_size, result->pw_name);
    printf("\nValeur inutile: %s", result_false->pw_name);

    exit(EXIT_SUCCESS);
}
