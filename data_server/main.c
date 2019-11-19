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
  char attribut[32];
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
  char tmp_attribut[N];
  char var_type[32];
  char *strToken;
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
  strToken = strtok(buff,":");
  strToken = strtok(NULL,"\n");
  strcpy(var_type,strToken);
  printf("var_type = %s",var_type);

  if(recvfrom(sock_fd, buff, N, 0, (struct sockaddr *)&server_addr, &tailleu) == -1)
  {
    perror("recvfrom");
    return 1;
  }
  printf("%s\n",buff);

  // il faut je reçois la liste des users !
  strToken = strtok(buff,":");
  printf("strtok = %s\n",strToken);
  int i=0;
  while(strToken != NULL)
  {
    strcpy(tab_user->table[i].login,strToken);
    strcpy(tab_user->table[i].attribut,"null");
    //memset((int*)&tab_user->table[i].attribut, -1, (size_t) N);
    i++;
    tab_user->nb_utilisateurs++;
    strToken = strtok (NULL,":");
  }
  printf("coucou\n");
  for(i=0; i < tab_user->nb_utilisateurs; i++)
  {
    printf("%d = %s\n",i, tab_user->table[i].login);
    printf("%d = %s\n",i, tab_user->table[i].attribut);
  }
  printf("dddddddddd\n");

  while(1)
  {
    memset((char *)&buff, 0, (size_t) N);
    if(recvfrom(sock_fd, buff, 1024, 0, (struct sockaddr *)&server_addr, &tailleu) == -1)
    {
      perror("recvfrom");
      return 1;
    }
    printf("buff = %s\n",buff);
    printf("ffffffffffffff\n");
    strToken = strtok(buff, " :\n");
    printf("toktok = %s %ld\n",strToken,strlen(strToken));
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
        if(strcmp(tab_user->table[i].attribut,"null") == 0)
        {
          sprintf(tmp_attribut,"%s:unknown\n",tab_user->table[i].login);
        }
        else
        {
          sprintf(tmp_attribut,"%s:%s\n",tab_user->table[i].login,tab_user->table[i].attribut);
        }
        strcat(buff,tmp_attribut);
      }
      printf("buff = %s",buff);
      if (sendto(sock_fd,buff,N, 0, (struct sockaddr *)&server_addr, tailleu) == -1)
      {
          perror("RIP");
          return 1;
      }
    }
    if(strncmp("ecrire",strToken,6) == 0)
    {
      char login[32]="";
      char type[32]="";
      char donnee[32]="";
      printf("je suis dans ecrire\n");
      printf("coucou");
      printf("nique reseau\n");
      strToken = strtok(NULL,".");
      strcpy(type,strToken);
      printf("type = %s",type);
      strToken= strtok(NULL,".");
      strcpy(donnee,strToken);
      printf("donnee = %s",donnee);
      strToken= strtok(NULL,"\n");
      strcpy(login,strToken);
      printf("login = %s",login);
      for(int i = 0; i < tab_user->nb_utilisateurs; i++)
      {
        if(strcmp(login,tab_user->table[i].login) == 0)
        {
            strcpy(tab_user->table[i].attribut,donnee);
            snprintf(buff,N,"ok pour %s\n",login);
            if (sendto(sock_fd,buff,N, 0, (struct sockaddr *)&server_addr, tailleu) == -1)
            {
                perror("RIP");
                return 1;
            }
        }
      }
      printf("fjdddddddd\n");
    }
    if(strcmp("supprimer",strToken) ==0)
    {
      char login[32]="";
      //strToken = strtok(NULL,".");
      strToken = strtok(NULL,"\n");
      strcpy(login,strToken);
      for(int i = 0; i < tab_user->nb_utilisateurs; i++)
      {
        printf("%s\n",tab_user->table[i].login);
        if(strcmp(login,tab_user->table[i].login) == 0)
        {
          int val_null = 0;
          //printf("iiiiiiii\n");
          if(strcmp(tab_user->table[i].attribut,"null") == 0)
          {
            val_null = 1;
            snprintf(buff,N,"Rien à supprimer dans %s pour %s\n",var_type,login);
            if (sendto(sock_fd,buff,N, 0, (struct sockaddr *)&server_addr, tailleu) == -1)
            {
                perror("RIP");
                return 1;
            }
          }
          if(val_null == 0)
          {
            strcpy(tab_user->table[i].attribut,"null");
            snprintf(buff,N,"La suppression de %s pour %s est ok\n",var_type,login);
            if (sendto(sock_fd,buff,N, 0, (struct sockaddr *)&server_addr, tailleu) == -1)
            {
                perror("RIP");
                return 1;
            }
          }
        }
      }
    }

  }
  printf("aaaaaaa\n");
  close(sock_fd);

  return EXIT_SUCCESS;
}
