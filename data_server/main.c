#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define N 1024

typedef struct
{
  char login[32];
  int age;
} User;

typedef struct
{
  User table[N];                      //tableau contenant UNE ligne du fichier à chaque indice i
  int nb_utilisateurs;     //nombre d'utilisateurs
} table_user;

int main(int argc, char**argv)
{

  if(argc!=4)
  {
    fprintf(stderr, "Usage %s <IP> <port> <nom>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  struct sockaddr_in server_addr;
  // Initialisation du tableau <user,age>
  table_user *tab_user = malloc(sizeof(table_user));
  tab_user->nb_utilisateurs = 0;

  int sock_fd;
  char buff[N];
  char tmp_age[N];
  char *strToken, *tmp;
  socklen_t tailleu = sizeof(struct sockaddr_in);

  //Création de socket
  sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock_fd == -1)
  {
    perror("Erreur socket()");
    return 1;
  }

  //Spécification addresse serveur
  memset( (char*) &server_addr, 0, sizeof(server_addr));//L'adresse est initialisé : on met des 0
  server_addr.sin_family = AF_INET; //Famille de l'adresse de destination
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(atoi(argv[2]));//Numéro de port de l'adresse de destination

  //Initialisation du buff à 0
  memset((char *)&buff, 0, (size_t) N);
  //Authentification
  snprintf(buff,1024,"server_auth %s",argv[3]);


  if (sendto(sock_fd,buff,N, 0, (struct sockaddr *)&server_addr, tailleu) == -1)
  {
      perror("RIP");
      return 1;
  }
  printf("%s\n",buff);
  //if pas ok fermer socket
  if(recvfrom(sock_fd, buff, N, 0, (struct sockaddr *)&server_addr, &tailleu) == -1)
  {
    perror("recvfrom");
    return 1;
  }
  printf("%s\n",buff);

  if(recvfrom(sock_fd, buff, N, 0, (struct sockaddr *)&server_addr, &tailleu) == -1)
  {
    perror("recvfrom");
    return 1;
  }
  printf("%s\n",buff);

  // il faut je reçois la liste des users !
  strToken = strtok_r(buff,":", &tmp);
  printf("strtok = %s\n",strToken);
  int i=0;
  while(strToken != NULL)
  {
    strcpy(tab_user->table[i].login,strToken);
    memset((int*)&tab_user->table[i].age, -1, (size_t) N);
    i++;
    tab_user->nb_utilisateurs++;
    strToken = strtok_r (NULL,":", &tmp);
  }
  printf("coucou\n");
  for(i=0; i < tab_user->nb_utilisateurs; i++)
  {
    printf("%d = %s\n",i, tab_user->table[i].login);
    printf("%d = %d\n",i, tab_user->table[i].age);
  }
  printf("dddddddddd\n");
  while(1)
  {
    printf("bbbbbbbbbb\n");
    memset((char *)&buff, 0, (size_t) N);
    printf("ccccccccc\n");
    if(recvfrom(sock_fd, buff, 1024, 0, (struct sockaddr *)&server_addr, &tailleu) == -1)
    {
      perror("recvfrom");
      return 1;
    }
    printf("buff = %s\n",buff);
    printf("ffffffffffffff\n");
    strToken = strtok_r(buff, " :", &tmp);
    printf("toktok = %s\n",strToken);
    // if(strcmp(NULL,strToken) == 0)
    // {
    //   printf("RIP\n");
    //   return 1;
    // }
    if(strncmp("lire",strToken,4) == 0)
    {
      printf("ggggggggggggggg\n");
      memset((char *)&buff, 0, (size_t) N);
      for(int i = 0; i < tab_user->nb_utilisateurs; i++)
      {
        if(tab_user->table[i].age == -1)
        {
          sprintf(tmp_age,"%s:unknown\n",tab_user->table[i].login);
        }
        else
        {
          sprintf(tmp_age,"%s:%u\n",tab_user->table[i].login,tab_user->table[i].age);
        }
        strcat(buff,tmp_age);
      }
      printf("buff = %s",buff);
      if (sendto(sock_fd,buff,N, 0, (struct sockaddr *)&server_addr, tailleu) == -1)
      {
          perror("RIP");
          return 1;
      }
    }

  }
  printf("aaaaaaa\n");
  close(sock_fd);

  return EXIT_SUCCESS;
}
