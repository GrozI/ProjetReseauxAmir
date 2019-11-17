#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "acces_server.c"

int main(int argc, char** argv)
{
  struct sockaddr_in server_addr,client_addr;
  int sock_fd;
  ssize_t size_message = 0;
  char buff[N];
  char *strToken, *tmp;
  unsigned int i = 0;
  socklen_t server_size = sizeof(struct sockaddr_in);
  socklen_t client_size = sizeof(struct sockaddr_in);
  int nombre_server = 0;
  int nb_user_connect = 0;
  //Ouverture du fichier où l'on stocke les informations dans le tableau
  //tab_userpuis fermeture du fichier
  table_user *tab_user = open_file();

  //Tableau qui sera la liste des serveurs de données
  table_server *list_data_server = malloc(N*sizeof(table_server));

  table_user_connect *list_table_user = malloc(N*sizeof(table_user_connect));

  // User *tab_user_connect = malloc(N*sizeof(User));
  // for(unsigned int p = 0; p < tab_user->nb_utilisateurs; p++)
  // {
  //   tab_user_connect = ajout_tab(tab_user_connect,tab_user->table[p].login);
  //   printf("%d = %s\n",i, tab_user_connect[i].login);
  // }
  if(argc != 2)
  {
    fprintf(stderr, "Usage %s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  printf("***** Serveur en ligne *****\n\n");

  //Création de la socket
  sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock_fd == -1)
  {
    perror("RIP SOCK2");
    return 1;
  }

  //Spécification de notre adresse
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(argv[1]));
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  //On associe la socket à l'adresse
  if (bind(sock_fd, (struct sockaddr *)&server_addr, server_size) == -1)
  {
    perror("RIP BIND");
    return 1;
  }

  //Petit affichage pour voir ce qui est contenu dans le tableau
  for(i=0; i < tab_user->nb_utilisateurs; i++)
  {
    printf("%d = %s\n",i, tab_user->table[i].login);
    printf("%d = %s\n",i, tab_user->table[i].mdp);
  }

  while(1)
  {
    //A chaque fois on remet le buff à 0
    memset((char *)&buff, 0, (size_t) N);

    //Réception du message du client ou bien du serveur de données
    if (recvfrom(sock_fd, buff, N, 0, (struct sockaddr *)&client_addr, &client_size) == -1)
    {
      perror("recvfrom");
      return 1;
    }
    //On affiche le contenu du message pour vérifier si c'est bien ce qu'on a envoyé
    printf("%s\n",buff);
    //On prend le premier mot du message et on l'affiche
    strToken = strtok_r(buff, " :", &tmp);
    printf("strtok = %s\n", strToken);
    printf("buffer = %s\n", tmp);
    //Si strToken est égale à auth alors je rentre dans la boucle
    if( strncmp("auth",strToken,4) == 0 && parcourir(sock_fd, buff,client_addr,list_table_user,list_data_server,nb_user_connect,nombre_server) == 0)
    {

      //comparer la liste des utilisateurs déjà connecté, si l'user est dans la liste
      //on envoit vous êtes déjà connecté et on break


      //Connecte nous dit quand l'utilisateur est connecte ou non
      int connecte = 0;
      //On prend le prochain mot qui est le login et on le stock dans une var tmp
      //puis on va parcourir le tableau pour voir si user est dedans ou non
      //et on envoi le message
      strToken = strtok_r (NULL,":", &tmp);
      printf("coucou  je suis : %s\n",strToken);
      char tmp_login[N]="";
      strncpy(tmp_login,strToken,strlen(strToken));
      printf("strtok = %s\n",tmp_login);
      strToken = strtok_r (NULL,":", &tmp);
      for(i=0; i < tab_user->nb_utilisateurs; i++)
      {
        // printf("%d = %s\n",i, tab_user->table[i].login);
        // printf("%d = %s\n",i, tab_user->table[i].mdp);
        // printf("%s\n",tmp_login);
        // printf("%s\n",strToken);

        if( (strcmp(tmp_login,tab_user->table[i].login) == 0) && (strcmp(strToken,tab_user->table[i].mdp) ==0))
        {
          printf("logintable = %s\n",tab_user->table[i].login);
          printf("mdptable = %s\n",tab_user->table[i].mdp);
          printf("tmplog = %s\n",tmp_login);
          snprintf(buff,N,"OK: vous êtes connectés au serveur !\n");
          if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
          {
            perror("erreur sockett sendefrrqz");
            exit(EXIT_FAILURE);
          }
          printf("tmplog2 = %s\n",tmp_login);
          connecte = 1;
          list_table_user[nb_user_connect].addr = client_addr;
          strcpy(list_table_user[nb_user_connect].login,tmp_login);
          nb_user_connect++;
          affiche_user(list_table_user,nb_user_connect);
          break;
        }
      }

      if(connecte == 0)
      {
        snprintf(buff,N,"PAS OK: vous n'êtes connectés au serveur car vos identifiants ne sont pas dans la base!\n");
        if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
        {
          perror("erreur sockett sendefrrqz");
          exit(EXIT_FAILURE);
        }
      }
    }

    //Si strToken est égale à bye alors je rentre dans la boucle
    if( strncmp("bye",strToken,3) == 0)
    {
      snprintf(buff,N,"bye!\n");
      if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
      {
        perror("erreur sockett sendefrrqz");
        exit(EXIT_FAILURE);
      }
      printf("%s\n",buff);
    }

    //Si strToken est égale à server_auth alors je rentre dans la boucle
    if( strcmp("server_auth",strToken) == 0 && parcourir(sock_fd, buff,client_addr,list_table_user,list_data_server,nb_user_connect,nombre_server) == 0)
    {
      strToken = strtok_r (NULL," :", &tmp);
      printf("%s\n",strToken);
      //printf("%ld\n",strlen(strToken));
      char tmp2[N]="";
      if(strncmp("age",strToken,3) == 0)
      {
          strncpy(tmp2,strToken,3);
          // printf("StrToken en rentrant la boucle strcmp et avant le snprintf : %s\n",strToken);
          snprintf(buff,N+1024,"Ok tu es le serveur : %s \n",tmp2);
          // printf("StrToken apres le snprintf : %s\n",strToken);
          //printf("buff = %s\n",buff);
          // printf("toz\n");
          if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
          {
            perror("erreur sockett sendefrrqz");
            exit(EXIT_FAILURE);
          }
          //printf("coucou\n");
          list_data_server[nombre_server].addr = client_addr;
          strcpy(list_data_server[nombre_server].type,"age");
          // printf("nbs : %d\n",nombre_server);
          // printf("port = %d\n",list_data_server[nombre_server].addr.sin_port);
          // printf("type = %s\n",list_data_server[nombre_server].type);
          nombre_server++;
          affiche(list_data_server,nombre_server);
      }
      printf("%s\n",tmp2);
      if(strcmp("taille",strToken) == 0)
      {
        printf("tailletaile\n");
          // list_data_server[nombre_server].addr = client_addr;
          // strcpy(list_data_server[nombre_server].type,"taille");
          // nombre_server++;
      }
      memset((char *)&buff, 0, (size_t) N);
      for(i=0; i < tab_user->nb_utilisateurs; i++)
      {
        for(unsigned int j = 0; j< tab_user->table[i].taille_attributs; j++)
        {
          if(strcmp(tmp2,tab_user->table[i].attribut[j]) == 0)
          {
            strcat(buff,tab_user->table[i].login);
            printf("%d = %s\n",i, tab_user->table[i].login);
            strcat(buff,":");
          }
        }
      }
      printf("%s\n",buff);
      if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
      {
        perror("erreur sockett sendefrrqz");
        exit(EXIT_FAILURE);
      }
    }
    if(strcmp("lire",strToken) == 0)
    {
      char tmp_lire[N]="";
      affiche_user(list_table_user,nb_user_connect);
      strToken = strtok_r (NULL," :\n", &tmp);
      printf("strttttok = %s\n",strToken);
      //on regarde le port du client pour le trouver dans la liste des users connect
      for(int i=0; i < nb_user_connect; i++)
      {
        if(client_addr.sin_port == list_table_user[i].addr.sin_port)
        {
          printf("port ok\n");
          //char log[32] = list_table_user[i].login;
          // a l'aide du port on trouve l'uilistaeur et on regarde maintenan si le login est dans le tableau qui a stock les login
          for(unsigned int j = 0; j < tab_user->nb_utilisateurs; j++)
          {
            if(strcmp(list_table_user[i].login,tab_user->table[j].login) == 0)
            {
              printf("login ok\n");
              //on parcourt la liste des attributs de l'user pour voir si il a le champ dans sa liste des attributs
              for(unsigned int k = 0; k < tab_user->table[j].taille_attributs; k++)
              {
                printf("log = %s\n",tab_user->table[j].login);
                printf("attr = %s %ld\n",tab_user->table[j].attribut[k],strlen(tab_user->table[j].attribut[k]));
                printf("kkkk = %s %ld\n", strToken, strlen(strToken));
                if(strncmp(strToken,tab_user->table[j].attribut[k],strlen(strToken)) == 0)
                {
                  printf("toktok = %s\n",strToken);
                  printf("okokokokokokok\n");
                  strncpy(tmp_lire,strToken,strlen(strToken));
                  printf("tmp_lire = %s\n",tmp_lire);
                  //envoyer au client qui a le droit de lire l'attribut
                  snprintf(buff,N+1024,"OK vous avez le droit de lire %s\n",tmp_lire);
                  if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                  {
                    perror("erreur sockett sendefrrqz");
                    exit(EXIT_FAILURE);
                  }
                  //envoyer au BON serveur de donnée la requêtre
                  for(int h = 0; h < nombre_server; h++)
                  { //envoi de la commande et attente du resultat
                    if(strcmp(list_data_server[h].type,tmp_lire) == 0)
                    {
                      printf("okokokokokokok2\n");
                      snprintf(buff,N+1024,"lire %s\n",tmp_lire);
                      client_addr.sin_port = list_data_server[h].addr.sin_port;
                      if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                      {
                        perror("erreur sockett sendefrrqz");
                        exit(EXIT_FAILURE);
                      }

                      if (recvfrom(sock_fd, buff, N, 0, (struct sockaddr *)&client_addr, &client_size) == -1)
                      {
                        perror("recvfrom");
                        return 1;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      // snprintf(buff,N,"PAS OK vous ne pouvez pas lire %s\n",strToken);
      // if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
      // {
      //   perror("erreur sockett sendefrrqz");
      //   exit(EXIT_FAILURE);
      // }
    }

  }

  printf("***** Serveur hors ligne *****\n\n");

  return 0;
}
