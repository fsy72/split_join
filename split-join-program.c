#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define BUFFER_SIZE 4096

void split(const char *nom_fichier, size_t c_size) {
    int fd = open(nom_fichier, O_RDONLY); // ouverture du fichier source
    if (fd < 0) {
        perror("Erreur d'ouverture du fichier source");
        exit(EXIT_FAILURE);
    }

    char buffer[c_size];
    size_t octets_lu;
    int numbre_part = 1;

    while ((octets_lu = read(fd, buffer, c_size)) > 0) {
        char part_name[30];
        snprintf(part_name, sizeof(part_name), "part%d_%s", numbre_part, nom_fichier);

        int part_fd = open(part_name, O_WRONLY | O_CREAT | O_TRUNC, 0644); // ouverture en écriture seul du fichier "derivé" n° : taille max c_size, en octal avec rw- r-- r--
        if (part_fd < 0) {
            perror("Erreur d'ouverture du fichier de sortie");
            close(fd);
            exit(EXIT_FAILURE);
        }

        size_t bytes_written = write(part_fd, buffer, octets_lu); // écriture sur le fichier "derivé" n°
        if (bytes_written != octets_lu) {
            perror("Erreur d'écriture");
            close(fd);
            close(part_fd);
            exit(EXIT_FAILURE);
        }

        close(part_fd);

        numbre_part++;
    }

    if (octets_lu < 0) {
        perror("Erreur de lecture");
    }

    close(fd);
    printf("Fichier découpé avec succès en %d parties.\n", numbre_part - 1);
}

void join(const char* fichier, int argc, char* argv[]) {
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int output = open(fichier, O_WRONLY | O_CREAT | O_TRUNC, mode); // ouverture en écriture seul du fichier de sorti
    if (output == -1) {
        perror("Erreur: Impossible de créer le fichier de sortie");
        exit(-1);
    }

    char buffer[BUFFER_SIZE];
    size_t octets_lu;

    for (int i = 2; i < argc; i++) {
        int input = open(argv[i], O_RDONLY);
        if (input == -1) {
            perror("Erreur: Impossible d'ouvrir un fichier partie");
            close(output);
            exit(-1);
        }

        // Copie le contenu
        while ((octets_lu = read(input, buffer, sizeof(buffer))) > 0) {
            if (write(output, buffer, octets_lu) != octets_lu) {
                perror("Erreur d'écriture");
                close(input);
                close(output);
                exit(-1);
            }
        }

        if (octets_lu == -1) {
            perror("Erreur de lecture");
            close(input);
            close(output);
            exit(-1);
        }

        close(input);
    }

    close(output);
    printf("Fichiers joints avec succès\n");
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage:\n");
        printf("Pour découper: %s <fichier> <taille>\n", argv[0]);
        printf("Pour joindre: %s <fichier_sortie> <part1> <part2> ...\n", argv[0]);
        return 1;
    }
    if (argc == 3) {
        size_t c_size = atoll(argv[2]);
        if (c_size == 0) {
            printf("Erreur: Taille invalide\n");
            return 1;
        }
        split(argv[1], c_size);
    }
    else {
        join(argv[1], argc, argv);
    }

    return 0;
}
