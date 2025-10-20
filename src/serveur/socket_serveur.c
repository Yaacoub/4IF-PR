/* src/server/server.c
 *
 * Serveur basé sur select() pour gérer:
 *  - enregistrement des pseudos
 *  - LIST des utilisateurs en ligne
 *  - CHALLENGE / ACCEPT / REFUSE
 *  - création de match et choix aléatoire du joueur qui commence
 *
 * Commentaires et noms de variables en français (comme demandé).
 *
 * Compilation: via ton Makefile (bin/server).
 */

#define _POSIX_C_SOURCE 200112L
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>  // pour strcasecmp()
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAX_CLIENTS 64
#define PSEUDO_MAX 32
#define BUF_SZ 4096
#define TRUE 1
#define FALSE 0

/* Structure client */
typedef struct {
    int fd;                       /* socket fd, -1 si libre */
    char pseudo[PSEUDO_MAX];      /* pseudo enregistré, vide si non enregistré */
    char buffer[BUF_SZ];          /* buffer d'entrée par lignes */
    int buf_pos;                  /* position actuelle dans le buffer */
    int en_match;                 /* 0 si libre, 1 si en partie */
    int adversaire_fd;            /* fd de l'adversaire si en_match */
} client_t;

/* Structure partie basique */
typedef struct {
    int fd_a;
    int fd_b;
    int fd_tour; /* fd du joueur dont c'est le tour */
} match_t;

/* tableau de clients */
static client_t clients[MAX_CLIENTS];
static match_t matches[MAX_CLIENTS/2]; /* au plus MAX_CLIENTS/2 matches */

/* helper: envoyer msg (ajoute \n automatiquement si pas presente) */
static ssize_t envoyer_ligne(int fd, const char *msg) {
    size_t len = strlen(msg);
    char tmp[1024];
    if (len + 2 < sizeof tmp) {
        strcpy(tmp, msg);
        if (len == 0 || tmp[len-1] != '\n') strcat(tmp, "\n");
        return write(fd, tmp, strlen(tmp));
    } else {
        /* grande ligne, envoye directement (supposons qu'elle contient \n) */
        return write(fd, msg, len);
    }
}

/* trouve index client par fd */
static int index_par_fd(int fd) {
    for (int i = 0; i < MAX_CLIENTS; ++i) if (clients[i].fd == fd) return i;
    return -1;
}

/* trouve index client par pseudo */
static int index_par_pseudo(const char *pseudo) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].fd != -1 && clients[i].pseudo[0] != '\0' &&
            strcmp(clients[i].pseudo, pseudo) == 0) return i;
    }
    return -1;
}

/* initialiser tableau de clients */
static void init_clients(void) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients[i].fd = -1;
        clients[i].pseudo[0] = '\0';
        clients[i].buf_pos = 0;
        clients[i].en_match = 0;
        clients[i].adversaire_fd = -1;
    }
    for (int i = 0; i < MAX_CLIENTS/2; ++i) {
        matches[i].fd_a = matches[i].fd_b = matches[i].fd_tour = -1;
    }
}

/* ajoute un nouveau client, retourne fd_index ou -1 */
static int ajouter_client(int fd) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].fd == -1) {
            clients[i].fd = fd;
            clients[i].pseudo[0] = '\0';
            clients[i].buf_pos = 0;
            clients[i].en_match = 0;
            clients[i].adversaire_fd = -1;
            return i;
        }
    }
    return -1;
}

/* supprime client */
static void supprimer_client(int idx) {
    if (idx < 0 || idx >= MAX_CLIENTS) return;
    int fd = clients[idx].fd;
    if (fd != -1) close(fd);
    clients[idx].fd = -1;
    clients[idx].pseudo[0] = '\0';
    clients[idx].buf_pos = 0;
    clients[idx].en_match = 0;
    clients[idx].adversaire_fd = -1;
}

/* broadcast simple (pour debug) */
static void broadcast(const char *msg) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].fd != -1) envoyer_ligne(clients[i].fd, msg);
    }
}

/* creer une match (retourne 0 si ok) */
static int creer_match(int fd_a, int fd_b) {
    for (int i = 0; i < MAX_CLIENTS/2; ++i) {
        if (matches[i].fd_a == -1) {
            matches[i].fd_a = fd_a;
            matches[i].fd_b = fd_b;
            /* decide aleatoirement qui commence */
            if (rand() % 2 == 0) matches[i].fd_tour = fd_a;
            else matches[i].fd_tour = fd_b;
            /* marque clients */
            int ia = index_par_fd(fd_a), ib = index_par_fd(fd_b);
            if (ia >= 0 && ib >= 0) {
                clients[ia].en_match = 1;
                clients[ia].adversaire_fd = fd_b;
                clients[ib].en_match = 1;
                clients[ib].adversaire_fd = fd_a;
            }
            return 0;
        }
    }
    return -1;
}

/* supprimer match que contient fd (si existe) */
static void supprimer_match_par_fd(int fd) {
    for (int i = 0; i < MAX_CLIENTS/2; ++i) {
        if (matches[i].fd_a == fd || matches[i].fd_b == fd) {
            int a = matches[i].fd_a, b = matches[i].fd_b;
            int ia = index_par_fd(a), ib = index_par_fd(b);
            if (ia >= 0) { clients[ia].en_match = 0; clients[ia].adversaire_fd = -1; }
            if (ib >= 0) { clients[ib].en_match = 0; clients[ib].adversaire_fd = -1; }
            matches[i].fd_a = matches[i].fd_b = matches[i].fd_tour = -1;
        }
    }
}

/* trouve match por fd (retorna index o -1) */
static int trouver_match_par_fd(int fd) {
    for (int i = 0; i < MAX_CLIENTS/2; ++i) {
        if (matches[i].fd_a == fd || matches[i].fd_b == fd) return i;
    }
    return -1;
}

/* traitement d'une ligne reçue depuis client index 'idx' */
static void traiter_ligne(int idx, const char *ligne) {
    /* enlever \r\n si presentes */
    char copie[1024];
    strncpy(copie, ligne, sizeof(copie)-1);
    copie[sizeof(copie)-1] = '\0';
    size_t L = strlen(copie);
    while (L > 0 && (copie[L-1] == '\n' || copie[L-1] == '\r')) { copie[L-1] = '\0'; --L; }

    int fd = clients[idx].fd;

    if (L == 0) return;

    /* Tokenisation simple */
    char cmd[64], arg[128];
    cmd[0] = arg[0] = '\0';
    int n = sscanf(copie, "%63s %127[^\n]", cmd, arg);

    /* COMMANDES */
    if (strcasecmp(cmd, "REGISTER") == 0) {
        if (n < 2) {
            envoyer_ligne(fd, "ERR Usage: REGISTER <pseudo>");
            return;
        }
        /* verifier unicite */
        if (index_par_pseudo(arg) != -1) {
            envoyer_ligne(fd, "ERR Pseudo deja pris");
            return;
        }
        strncpy(clients[idx].pseudo, arg, PSEUDO_MAX-1);
        clients[idx].pseudo[PSEUDO_MAX-1] = '\0';
        char msg[128];
        snprintf(msg, sizeof(msg), "OK Registered as %s", clients[idx].pseudo);
        envoyer_ligne(fd, msg);
        return;
    } else if (strcasecmp(cmd, "LIST") == 0) {
        /* envoyer lista de usuarios */
        envoyer_ligne(fd, "OK Users:");
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i].fd != -1 && clients[i].pseudo[0] != '\0') {
                char ligneu[128];
                snprintf(ligneu, sizeof(ligneu), "%s%s", clients[i].pseudo,
                         clients[i].en_match ? " (inmatch)" : "");
                envoyer_ligne(fd, ligneu);
            }
        }
        envoyer_ligne(fd, "OK EndList");
        return;
    } else if (strcasecmp(cmd, "CHALLENGE") == 0) {
        if (n < 2) { envoyer_ligne(fd, "ERR Usage: CHALLENGE <pseudo>"); return; }
        int cible_idx = index_par_pseudo(arg);
        if (cible_idx == -1) { envoyer_ligne(fd, "ERR User not found"); return; }
        if (clients[cible_idx].en_match) { envoyer_ligne(fd, "ERR Target already in match"); return; }
        if (clients[idx].en_match) { envoyer_ligne(fd, "ERR You are already in match"); return; }
        /* envoyer invitation al target */
        char invit[256];
        snprintf(invit, sizeof(invit), "INVT %s %s", clients[idx].pseudo, clients[idx].pseudo);
        /* formato: INVT <from> <message?> - simplified */
        envoyer_ligne(clients[cible_idx].fd, invit);
        envoyer_ligne(fd, "OK Challenge sent");
        return;
    } else if (strcasecmp(cmd, "ACCEPT") == 0) {
        if (n < 2) { envoyer_ligne(fd, "ERR Usage: ACCEPT <pseudo>"); return; }
        int auteur = index_par_pseudo(arg);
        if (auteur == -1) { envoyer_ligne(fd, "ERR No such user"); return; }
        if (clients[idx].en_match || clients[auteur].en_match) {
            envoyer_ligne(fd, "ERR One of users already in match");
            return;
        }
        /* crear match */
        if (creer_match(clients[auteur].fd, clients[idx].fd) == 0) {
            /* notificar ambos: START <opponent> <who_starts_pseudo> */
            int ia = index_par_fd(clients[auteur].fd);
            int ib = idx;
            int m = trouver_match_par_fd(clients[auteur].fd);
            if (m >= 0) {
                int starter_fd = matches[m].fd_tour;
                const char *starter_pseudo = (starter_fd == clients[auteur].fd) ? clients[ia].pseudo : clients[ib].pseudo;
                char msg_a[256], msg_b[256];
                snprintf(msg_a, sizeof(msg_a), "START %s %s", clients[ib].pseudo, starter_pseudo);
                snprintf(msg_b, sizeof(msg_b), "START %s %s", clients[ia].pseudo, starter_pseudo);
                envoyer_ligne(clients[ia].fd, msg_a);
                envoyer_ligne(clients[ib].fd, msg_b);
            }
            return;
        } else {
            envoyer_ligne(fd, "ERR Unable to create match (server full)");
            return;
        }
    } else if (strcasecmp(cmd, "REFUSE") == 0) {
        if (n < 2) { envoyer_ligne(fd, "ERR Usage: REFUSE <pseudo>"); return; }
        int auteur = index_par_pseudo(arg);
        if (auteur == -1) { envoyer_ligne(fd, "ERR No such user"); return; }
        /* notificar al autor */
        char msg[128];
        snprintf(msg, sizeof(msg), "OK %s refused your challenge", clients[idx].pseudo);
        envoyer_ligne(clients[auteur].fd, msg);
        envoyer_ligne(fd, "OK Refused");
        return;
    } else if (strcasecmp(cmd, "QUIT") == 0) {
        envoyer_ligne(fd, "OK Bye");
        /* cierre del fd gestionado por bucle superior */
        close(fd);
        clients[idx].fd = -1;
        clients[idx].pseudo[0] = '\0';
        /* limpiar match si estaba en uno */
        supprimer_match_par_fd(fd);
        return;
    } else {
        envoyer_ligne(fd, "ERR Unknown command");
        return;
    }
}

/* lecture de données du client, remplir buffer y procesar lineas completas */
static void lire_client(int idx) {
    int fd = clients[idx].fd;
    if (fd < 0) return;
    ssize_t n = read(fd, clients[idx].buffer + clients[idx].buf_pos,
                     sizeof(clients[idx].buffer) - clients[idx].buf_pos - 1);
    if (n <= 0) {
        /* desconexion */
        if (n == 0) {
            printf("client fd=%d closed connection\n", fd);
        } else {
            perror("read client");
        }
        /* limpiar partido si existia */
        supprimer_match_par_fd(fd);
        supprimer_client(idx);
        return;
    }
    clients[idx].buf_pos += (int)n;
    clients[idx].buffer[clients[idx].buf_pos] = '\0';

    /* procesar lineas completas */
    char *start = clients[idx].buffer;
    char *newline;
    while ((newline = strchr(start, '\n')) != NULL) {
        size_t linelen = (newline - start) + 1;
        char linea[1024];
        if (linelen >= sizeof linea) linelen = sizeof linea - 1;
        memcpy(linea, start, linelen);
        linea[linelen] = '\0';
        traiter_ligne(idx, linea);
        start = newline + 1;
    }

    /* desplazar datos incompletos al inicio del buffer */
    size_t rem = strlen(start);
    memmove(clients[idx].buffer, start, rem + 1);
    clients[idx].buf_pos = (int)rem;
}

/* main: config server y loop select */
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);

    /* seed pour aleatoire */
    srand((unsigned int)time(NULL));

    init_clients();

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }

    int oui = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &oui, sizeof(oui));

    struct sockaddr_in adr;
    memset(&adr, 0, sizeof adr);
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons((unsigned short)port);

    if (bind(listen_fd, (struct sockaddr*)&adr, sizeof adr) < 0) { perror("bind"); return 1; }
    if (listen(listen_fd, 16) < 0) { perror("listen"); return 1; }

    printf("serveur select: j'écoute sur le port %d\n", port);

    fd_set readfds;
    while (TRUE) {
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);
        int maxfd = listen_fd;

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i].fd != -1) {
                FD_SET(clients[i].fd, &readfds);
                if (clients[i].fd > maxfd) maxfd = clients[i].fd;
            }
        }

        int ret = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        /* nouvelle connexion */
        if (FD_ISSET(listen_fd, &readfds)) {
            int connfd = accept(listen_fd, NULL, NULL);
            if (connfd < 0) { perror("accept"); }
            else {
                int idx = ajouter_client(connfd);
                if (idx == -1) {
                    envoyer_ligne(connfd, "ERR Server full");
                    close(connfd);
                } else {
                    envoyer_ligne(connfd, "OK Welcome. Please REGISTER <pseudo>");
                    printf("Nouveau client fd=%d idx=%d\n", connfd, idx);
                }
            }
        }

        /* lecture clients */
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i].fd != -1 && FD_ISSET(clients[i].fd, &readfds)) {
                lire_client(i);
            }
        }
    }

    close(listen_fd);
    return 0;
}
