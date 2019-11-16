#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "client.c"


int main(int argc, char**argv)
{
  if(argc!=5)
  {
    fprintf(stderr, "Usage %s <IP> <port> <login> <mdp>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  struct sockaddr_in server_addr;

  int sock_fd;
  char buff[N];
  char *strToken, *tmp;
  struct timeval timeout;
  socklen_t tailleu = sizeof(struct sockaddr_in);


  sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock_fd == -1)
  {
    perror("Erreur socket()");
    return 1;
  }

  memset( (char*) &server_addr, 0, sizeof(server_addr));//L'adresse est initialisé : on met des 0
  server_addr.sin_family = AF_INET; //Famille de l'adresse de destination
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(atoi(argv[2]));//Numéro de port de l'adresse de destination

  memset((char *)&buff, 0, (size_t) N);
  snprintf(buff,1024,"auth %s:%s",argv[3],argv[4]);

  timeout.tv_sec = 2;
  timeout.tv_usec = 0;

  if (sendto(sock_fd,buff,N, 0, (struct sockaddr *)&server_addr, tailleu) == -1)
  {
      perror("RIP");
      return 1;
  }
  //if pas ok fermer socket
  fd_set ensemble;
  int max = sock_fd+1;
  while(1)
  {
    memset((char *)&buff, 0, (size_t) N);
    FD_ZERO(&ensemble);
    FD_SET(sock_fd, &ensemble);
    FD_SET(0, &ensemble);
    int sel = select(max, &ensemble, NULL, NULL, NULL);
    switch(sel)
    {
      case -1:
        printf("RIP\n");
        return 1;
      case 0:
        printf("Serveur hors ligne\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
      default:

          if (FD_ISSET(sock_fd, &ensemble))
          {

            recvfrom(sock_fd, buff, 1024, 0, (struct sockaddr *)&server_addr, &tailleu);
            if(strncmp("bye",buff,3) == 0)
            {
              printf("%s\n", buff);
              close(sock_fd);
              exit(EXIT_SUCCESS);
            }
            if(strncmp("PAS OK",buff,6) == 0)
            {
              printf("%s", buff);
              close(sock_fd);
              exit(EXIT_SUCCESS);
            }

            printf("buff = %s\n", buff);
          }

          if(FD_ISSET(0,&ensemble))
          {
            if( read(0,buff,N) == -1)
            {
              perror("rip read");
              exit(EXIT_FAILURE);
            }
            if (sendto(sock_fd,buff,N, 0, (struct sockaddr *)&server_addr, tailleu) == -1)
            {
                perror("RIP");
                return 1;
            }
          }
    }

  }
  close(sock_fd);

  return EXIT_SUCCESS;
}
