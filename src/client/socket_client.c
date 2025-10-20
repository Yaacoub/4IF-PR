/* src/client/client.c
 *
 * Client interactif:
 *  - se connecte au serveur
 *  - lit stdin et envoie lignes (terminées par \n)
 *  - lit serveur et affiche les réponses
 *
 * Commentaires en français et noms de variables en français.
 *
 * Usage: ./bin/client <server_ip> <port>
 */

#define _POSIX_C_SOURCE 200112L
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof serv);
    serv.sin_family = AF_INET;
    serv.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, ip, &serv.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*)&serv, sizeof serv) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    printf("client: connecté à %s:%d\n", ip, port);
    printf("Tapez 'REGISTER <pseudo>' par exemple, ou 'LIST', 'CHALLENGE <pseudo>', 'ACCEPT <pseudo>'\n");

    fd_set readfds;
    char buf[4096];

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);       /* stdin */
        FD_SET(sockfd, &readfds);  /* socket */
        int maxfd = (sockfd > 0) ? sockfd : 0;

        int rc = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (rc < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        /* stdin prêt: lire ligne et envoyer */
        if (FD_ISSET(0, &readfds)) {
            if (fgets(buf, sizeof buf, stdin) == NULL) {
                /* EOF: close */
                printf("client: EOF stdin, fermeture\n");
                break;
            }
            size_t L = strlen(buf);
            if (L > 0) {
                ssize_t w = write(sockfd, buf, (ssize_t)L);
                if (w < 0) { perror("write to server"); break; }
            }
        }

        /* socket prêt: lire reponses du serveur */
        if (FD_ISSET(sockfd, &readfds)) {
            ssize_t n = read(sockfd, buf, sizeof buf - 1);
            if (n <= 0) {
                if (n == 0) printf("client: serveur a fermé la connexion\n");
                else perror("read server");
                break;
            }
            buf[n] = '\0';
            printf("serveur: %s", buf);
        }
    }

    close(sockfd);
    return 0;
}
