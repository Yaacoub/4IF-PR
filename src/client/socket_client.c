/* socket_client.c — client TCP simple qui envoie le contenu d'un fichier
 * Commentaires en français.
 *
 * Usage: ./client <server_ip> <port>
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char** argv )
{
  int sockfd;
  struct sockaddr_in serv_addr;
  FILE *file;
  char buffer[1024];
  size_t nread;

  if (argc != 3) {
    printf("usage: %s <server_ip> <port>\n", argv[0]);
    return 1;
  }

  printf("client: démarrage\n");

  /* Préparer l'adresse du serveur */
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_addr.sin_port = htons(atoi(argv[2]));

  /* ouvrir le socket */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return 1;
  }

  /* se connecter */
  if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("connect");
    close(sockfd);
    return 1;
  }

  /* Ouvrir le fichier data/tableu.txt et envoyer sa taille réelle */
  file = fopen("data/tableu.txt", "r");
  if (file == NULL) {
    perror("fopen data/tableu.txt");
    close(sockfd);
    return 1;
  }

  /* Lire le fichier par blocs et envoyer seulement les octets lus */
  while ((nread = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    ssize_t nw = write(sockfd, buffer, (ssize_t)nread);
    if (nw < 0) {
      perror("write");
      break;
    }
  }
  fclose(file);

  /* Ensuite, envoyer ce que l'utilisateur tape au clavier jusqu'à EOF */
  int ch;
  while ((ch = getchar()) != EOF) {
    unsigned char c = (unsigned char) ch;
    if (write(sockfd, &c, 1) < 0) {
      perror("write keyboard");
      break;
    }
  }

  close(sockfd);
  printf("client: terminé\n");
  return 0;
}
