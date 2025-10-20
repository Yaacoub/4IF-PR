/* socket_server.c — serveur multi-clients simple */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    return 1;
  }

  int port = atoi(argv[1]);
  int sd, scomm;
  struct sockaddr_in adr_serv;

  signal(SIGCHLD, SIG_IGN);               

  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0) { perror("socket"); return 1; }

  int oui = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &oui, sizeof(oui));

  memset(&adr_serv, 0, sizeof(adr_serv));
  adr_serv.sin_family      = AF_INET;
  adr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
  adr_serv.sin_port        = htons((unsigned short)port);

  if (bind(sd, (struct sockaddr *)&adr_serv, sizeof(adr_serv)) < 0) {
    perror("bind"); close(sd); return 1;
  }
  if (listen(sd, 16) < 0) {
    perror("listen"); close(sd); return 1;
  }

  printf("serveur: j’écoute sur le port %d…\n", port);

  while (1) {
    scomm = accept(sd, NULL, NULL); 
    //si c'etait interrompu par une signal (errno == EINTR) → reesayyer, sinon → imprime erreur et revient au boucle.
    if (scomm < 0) {
      if (errno == EINTR) continue;
      perror("accept");
      continue;
    }

    pid_t pid = fork();                // créer un fils pour chaque client
    if (pid < 0) {
      perror("fork");
      close(scomm);
      continue;
    }

    if (pid == 0) {                       // fils
      close(sd);                          // le fils n'accept des nouveaux
      printf("serveur(fils %d): client connecté.\n", getpid());

      // echo con prefijo PID
      char buf[1024];
      ssize_t n;
      while ((n = read(scomm, buf, sizeof buf)) > 0) {
        char prefix[64];
        int plen = snprintf(prefix, sizeof(prefix), "[PID=%d] ", getpid());
        if (plen > 0) write(scomm, prefix, (size_t)plen);
        write(scomm, buf, (size_t)n);
      }

      close(scomm);
      printf("serveur(fils %d): client déconnecté, je termine.\n", getpid());
      _exit(0);
    } else {                              // père
      close(scomm);                       // pere revient à l'accept
    }
  }

  close(sd);
  return 0;
}
