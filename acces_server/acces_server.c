#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define N 1024
#define TIMEOUT 2

typedef struct
{
  char login[32];
  char mdp[32];
  char attribut[32][32];
  unsigned int taille_attributs;        //nombre d'attributs d'un utilisateurs
} User;

//Structure contenant le tableau des utilisateurs (tu verras très vite que t'en auras encore besoin haha)
typedef struct
{
  User *table;                      //tableau contenant UNE ligne du fichier à chaque indice i
  unsigned int nb_utilisateurs;     //nombre d'utilisateurs
} table_user;

typedef struct
{
  struct sockaddr_in addr;
  char type[32];
} table_server;


int create_socket()
{
  int sock_fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_IP);
  if(sock_fd == -1)
  {
    perror("Erreur create_socket");
    exit(EXIT_FAILURE);
  }
  return sock_fd;
}

int close_fd(int fd)
{
  if( close(fd) == -1)
  {
    perror("Erreur close_fd");
    exit(EXIT_FAILURE);
  }
  return 0;
}


int bind_socket(int sock_fd,struct sockaddr_in server_addr)
{
  int bsock;
  bsock = bind(sock_fd, (struct sockaddr *) &server_addr,sizeof(server_addr));
  if(bsock == -1)
  {
    perror("Erreur bind_socket");
    exit(EXIT_FAILURE);
  }
  return bsock;
}

ssize_t recv_message(int sock_fd,char* buf, struct sockaddr_in client_addr)
{
  ssize_t message;
  socklen_t clientSize = sizeof(client_addr);
  message = recvfrom(sock_fd,buf,N,0,(struct sockaddr *) &client_addr, &clientSize);
  if(message == -1)
  {
    perror("Erreur recv_message");
    exit(EXIT_FAILURE);
  }
  return message;
}

table_user *open_file()
{
  FILE* fp;
  int fermeture;
  char line[512], *strToken, *tmp;
  unsigned int i;
  fp = fopen("data.txt","r");
  if(fp == NULL)
  {
    perror("Erreur fopen");
    exit(EXIT_FAILURE);
  }

  /*ICI je compte le nombre de ligne du fichier avant de commencer à le lire ensuite*/
  unsigned int nb_lignes = 0, compteur = 0;
  char c;
  while ((c = fgetc(fp)) != EOF)
  {
      if ((c == '\n') || (c == '\r'))
      {
        //3 car on peut avoir au minimum 3 char par ligne genre a:b par exemple ou a login et b mdp
          if (compteur >= 3)
              nb_lignes ++;
          compteur = 0;
      }
      else
          compteur ++;
  }
  //3 car on on peut avoir une seule ligne donc le fichier se terminera par EOF et pas par \n ou \r
  if (compteur >= 3)
      nb_lignes ++;

  table_user *tab = malloc(sizeof(table_user));
  if (tab == NULL)
  {
    perror("malloc error");
    return NULL;
  }

  tab->table = malloc(sizeof(User) * nb_lignes);
  if (tab->table == NULL)
  {
    perror("malloc error");
    return NULL;
  }

  //nb users = nb lignes
  tab->nb_utilisateurs = nb_lignes;

  //Je me repositionne au début du fichier car j'étais à la fin après avoir compter le nombre de lignes
  if (fseek(fp, 0, SEEK_SET) == -1)
  {
    perror("erreur fseeek");
    return NULL;
  }

  for (i = 0; i < nb_lignes; i++)
  {
    fgets(line, 512, fp);
    line[strlen(line) - 1] = '\0';
    strToken = strtok_r(line, ":", &tmp);
    strncpy(tab->table[i].login ,strToken, 32);     //32 arbitraire mets une constante
    strToken = strtok_r (NULL,":", &tmp);
    strncpy(tab->table[i].mdp , strToken, 32);
    unsigned int j = 0;
    while ((strToken = strtok_r(NULL, ":", &tmp)) != NULL)
    {
      strncpy(tab->table[i].attribut[j], strToken, 32);
      j++;
    }
    tab->table[i].taille_attributs = j;
  }

  fermeture = fclose(fp);
  if(fermeture == EOF)
  {
    perror("Erreur fclose");
    exit(EXIT_FAILURE);
  }

  return tab;
}


ssize_t send_message(int sock_fd,char*buf,struct sockaddr_in server_addr)
{
  ssize_t size_message = sendto(sock_fd,buf,N,0,(struct sockaddr *) &server_addr,sizeof(server_addr));
  if(size_message == -1)
  {
    perror("Erreur send_message");
    exit(EXIT_FAILURE);
  }
  return size_message;
}

// User vide_tab(User *tab)
// {
//   return *tab == NULL;
// }
//
// User ajout_tab(User *tab,char* x)
// {
//   int i;
//   for(i = 0; i < N; i++)
//   {
//     if(tab[i].login == NULL)
//     {
//       tab[i].login = x;
//     }
//   }
//   return tab;
// }

void affiche(int nombre_server)
{
  for(int i= 0; i < nombre_server; i++)
  {

  }
}
