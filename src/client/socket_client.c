/* Client pour les sockets
 *    socket_client ip_server port
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
  int    sockfd,newsockfd,clilen,chilpid,ok,nleft,nbwriten;
  char c;
  struct sockaddr_in cli_addr,serv_addr;
  int ch; /* EX1 */

  if (argc!=3) {printf ("usage  socket_client server port\n");exit(0);}
 
 
  /*
   *  partie client 
   */
  printf ("client starting\n");  

  /* initialise la structure de donnee */
  bzero((char*) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family       = AF_INET;
  serv_addr.sin_addr.s_addr  = inet_addr(argv[1]);
  serv_addr.sin_port         = htons(atoi(argv[2]));
  
  /* ouvre le socket */
  if ((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
    {printf("socket error\n");exit(0);}
  
  /* effectue la connection */
  if (connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {printf("socket error\n");exit(0);}
    
  
    /* TODO Ameliorer, et aussi mettre username*/
    FILE *file = fopen("data/tableu.txt", "r");
    if (file == NULL) {
      printf("Error opening file\n");
      exit(1);
    }
    char buffer[1024];
    fread(buffer, 1, 1024, file);
    fclose(file);
    write(sockfd, buffer, 1024);



  /* repete dans le socket tout ce qu'il entend */
  while ((ch = getchar()) != EOF) { /* EX1 */
    write(sockfd, &ch, 1);
  }

  close(sockfd);
  return 0;

  /* RUNNING: ./socket_client 127.0.0.1  9999
  */

}
