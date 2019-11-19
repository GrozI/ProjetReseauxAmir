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
  char buff[N];
  char *strToken;
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
    strToken = strtok(buff, " :\n");
    printf("strtoken = %s\n",strToken);
    // printf("strtok = %s\n", strToken);
    // printf("buffer = %s\n", tmp);
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
      strToken = strtok(NULL,":");
      // printf("coucou  je suis : %s\n",strToken);
      char tmp_login[N]="";
      strncpy(tmp_login,strToken,strlen(strToken));
      // printf("strtok = %s\n",tmp_login);
      strToken = strtok(NULL,":");
      for(i=0; i < tab_user->nb_utilisateurs; i++)
      {
        // printf("%d = %s\n",i, tab_user->table[i].login);
        // printf("%d = %s\n",i, tab_user->table[i].mdp);
        // printf("%s\n",tmp_login);
        // printf("%s\n",strToken);

        if( (strcmp(tmp_login,tab_user->table[i].login) == 0) && (strcmp(strToken,tab_user->table[i].mdp) ==0))
        {
          // printf("logintable = %s\n",tab_user->table[i].login);
          // printf("mdptable = %s\n",tab_user->table[i].mdp);
          // printf("tmplog = %s\n",tmp_login);
          snprintf(buff,N,"OK: vous êtes connectés au serveur !\n");
          if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
          {
            perror("erreur sockett sendefrrqz");
            exit(EXIT_FAILURE);
          }
          //printf("tmplog2 = %s\n",tmp_login);
          connecte = 1;
          //list_table_user[nb_user_connect].addr = client_addr;
          //&list_table_user[nb_user_connect].addr = malloc(sizeof(struct sockaddr_in));
          copie(&client_addr,&list_table_user[nb_user_connect].addr);
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
    else if( strncmp("bye",strToken,3) == 0)
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
    else if( strcmp("server_auth",strToken) == 0 && parcourir(sock_fd, buff,client_addr,list_table_user,list_data_server,nb_user_connect,nombre_server) == 0)
    {
      strToken = strtok(NULL," :");
      printf("%s\n",strToken);
      //printf("%ld\n",strlen(strToken));
      char tmp2[N]="";
      if(strncmp("age",strToken,3) == 0)
      {
          strncpy(tmp2,strToken,3);
          // printf("StrToken en rentrant la boucle strcmp et avant le snprintf : %s\n",strToken);
          snprintf(buff,N+1024,"Ok tu es le serveur :%s\n",tmp2);
          // printf("StrToken apres le snprintf : %s\n",strToken);
          //printf("buff = %s\n",buff);
          // printf("toz\n");
          if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
          {
            perror("erreur sockett sendefrrqz");
            exit(EXIT_FAILURE);
          }
          //printf("coucou\n");
          //&list_data_server[nombre_server].addr = malloc(sizeof(struct sockaddr_in));
          copie(&client_addr,&list_data_server[nombre_server].addr);
          //list_data_server[nombre_server].addr = client_addr;
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
        strncpy(tmp2,strToken,6);
        snprintf(buff,N+1024,"Ok tu es le serveur :%s\n",tmp2);
        if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
        {
          perror("erreur sockett sendefrrqz");
          exit(EXIT_FAILURE);
        }
        copie(&client_addr,&list_data_server[nombre_server].addr);
        strcpy(list_data_server[nombre_server].type,"taille");
        nombre_server++;
        affiche(list_data_server,nombre_server);
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
    else if(strcmp("lire",strToken) == 0)
    {
      char tmp_lire[N]="";
      char champ[32][32]={0};
      int cpt=0;
      affiche_user(list_table_user,nb_user_connect);
      strToken = strtok(NULL," \n");
      while(strToken != NULL)
      {
        printf("strttttok = %s\n",strToken);
        strcpy(champ[cpt],strToken);
        printf("champ : %s\n",champ[cpt]);
        strToken = strtok(NULL," \n");
        cpt++;
      }
      printf("test\n");
      //printf("strttttok = %s\n",strToken);
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
              int test = 0;
              for(unsigned int k = 0; k < tab_user->table[j].taille_attributs; k++)
              {
                // printf("log = %s\n",tab_user->table[j].login);
                // printf("attr = %s %ld\n",tab_user->table[j].attribut[k],strlen(tab_user->table[j].attribut[k]));
                //if(strncmp(strToken,tab_user->table[j].attribut[k],strlen(strToken)) == 0)
                for(int t = 0; t < cpt ; t++)
                {
                if(strcmp(champ[t],tab_user->table[j].attribut[k]) == 0)
                {
                  int serveur_connecte = 0; //tester si le serveur est co
                  int droit =0;
                  test = 1;
                  //printf("toktok = %s\n",strToken);
                  //strncpy(tmp_lire,strToken,strlen(strToken));
                  strcpy(tmp_lire,champ[t]);
                  printf("tmp_lire = %s\n",tmp_lire);
                  //envoyer au BON serveur de donnée la requêtre
                  for(int h = 0; h < nombre_server; h++)
                  { //envoi de la commande et attente du resultat
                    if(strcmp(list_data_server[h].type,tmp_lire) == 0)
                    {
                      if(droit == 0)
                      {
                        //envoyer au client qui a le droit de lire l'attribut
                        snprintf(buff,N+1024,"OK vous avez le droit de lire %s\n",tmp_lire);
                        if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                        {
                          perror("erreur sockett sendefrrqz");
                          exit(EXIT_FAILURE);
                        }
                      }
                      droit = 1;
                      serveur_connecte = 1;
                      affiche(list_data_server,nombre_server);
                      snprintf(buff,N+1024,"lire %s\n",tmp_lire);
                      printf("client addr port : %d\n", client_addr.sin_port);
                      //client_addr.sin_port = list_data_server[h].addr.sin_port;
                      printf("client addr port : %d\n", client_addr.sin_port);
                      printf("server port : %d\n",list_data_server[h].addr.sin_port);
                      printf("buff = %s\n",buff);
                      client_addr = list_data_server[h].addr;
                      if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                      {
                        perror("erreur sockett sendefrrqz");
                        exit(EXIT_FAILURE);
                      }
                      //printf("client addr port : %d\n", client_addr.sin_port);
                      if (recvfrom(sock_fd, buff, N, 0, (struct sockaddr *)&client_addr, &client_size) == -1)
                      {
                        perror("recvfrom");
                        return 1;
                      }
                      printf("%s\n",buff);

                      client_addr = list_table_user[i].addr;
                      if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                      {
                        perror("erreur sockett sendefrrqz");
                        exit(EXIT_FAILURE);
                      }
                      break;
                    }
                  }
                  if(serveur_connecte == 0)
                  {
                    snprintf(buff,N,"Le serveur que vous avez indiquer est indisponible\n");
                    if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                    {
                      perror("erreur sockett sendefrrqz");
                      exit(EXIT_FAILURE);
                    }
                  }
                }
              }
              }
              if(test == 0)
              {
                snprintf(buff,N,"Vous ne pouvez pas lire ce champ\n");
                if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                {
                  perror("erreur sockett sendefrrqz");
                  exit(EXIT_FAILURE);
                }
              }
            }
          }
        }
      }

    }
    else if(strcmp("ecrire",strToken) == 0)
    {
      //char tmp_type[32]=""; // pour avoir juste age
      char donnee[32][32]={0};
      affiche_user(list_table_user,nb_user_connect); //j'affiche la liste des users connectés
      char champ[32][32]={0};
      int cpt=0;
      //printf("tmp dans ecrire = %s",tmp);
      strToken = strtok (NULL," :");
      while(strToken != NULL)
      {
        printf("ecriretoktok = %s\n",strToken);
        strcpy(champ[cpt],strToken);
        printf("champ : %s\n",champ[cpt]);
        strToken = strtok(NULL," :\n");
        if(strToken != NULL)
        {
          strcpy(donnee[cpt],strToken);
          printf("donnee : %s\n",donnee[cpt]);
          strToken = strtok(NULL," :\n");
        }
        cpt++;
      }
      //printf("tmp v2 dans ecrire = %s",tmp);
      //printf("strToken = %s",strToken);
      //strcpy(tmp_type,strToken); // je mets le type age/taille etc dans tmp_type
      //printf("tmp_type = %s",tmp_type);

      //on regarde le port du client pour le trouver dans la liste des users connect
      for(int i=0; i < nb_user_connect; i++)
      {
        if(client_addr.sin_port == list_table_user[i].addr.sin_port)
        {
          // a l'aide du port on trouve l'uilistaeur et on regarde maintenan si le login est dans le tableau qui a stock les login
          for(unsigned int j = 0; j < tab_user->nb_utilisateurs; j++)
          {
            if(strcmp(list_table_user[i].login,tab_user->table[j].login) == 0)
            {
              //on parcourt la liste des attributs de l'user pour voir si il a le champ dans sa liste des attributs
              int test = 0;
              for(unsigned int k = 0; k < tab_user->table[j].taille_attributs; k++)
              {
                printf("log2 = %s\n",tab_user->table[j].login);
                printf("attr2 = %s %ld\n",tab_user->table[j].attribut[k],strlen(tab_user->table[j].attribut[k]));
                //printf("kkkk2 = %s %ld\n", tmp_type, strlen(tmp_type));
                for(int t =0; t < cpt; t++)
                {
                //if(strncmp(tmp_type,tab_user->table[j].attribut[k],strlen(tmp_type)) == 0)
                if(strcmp(champ[t],tab_user->table[j].attribut[k]) == 0)
                {
                  test = 1;
                  int serveur_connecte = 0; //tester si le serveur est co
                  int droit =0;
                  int envoi_ok = 0;
                  int count_type = 0;
                  printf("atoz\n");
                  //strToken = strtok(NULL,"\n");
                  //printf("strToken v2 = %s\n",strToken);
                  //strcpy(donnee,strToken); // je mets la donnée du type dans donnee
                  //printf("donnee = %s\n",donnee);
                  //envoyer au BON serveur de donnée la requêtre
                  for(int h = 0; h < nombre_server; h++)
                  { //envoi de la commande et attente du resultat
                    if(strcmp(list_data_server[h].type,champ[t]) == 0)
                    {
                      count_type++;
                      printf("count_type %d\n",count_type);
                      //On donne l'autorisation au client
                      if(droit == 0)
                      {
                        snprintf(buff,N,"OK vous avez le droit d'ecrire %s\n",champ[t]);
                        if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                        {
                          perror("erreur sockett sendefrrqz");
                          exit(EXIT_FAILURE);
                        }
                      }
                      droit = 1;
                      serveur_connecte = 1;
                      affiche(list_data_server,nombre_server);
                      snprintf(buff,N+2048,"ecrire %s.%s.%s\n",champ[t],donnee[t],tab_user->table[j].login);
                      //printf("client addr port : %d\n", client_addr.sin_port);
                      //printf("client addr port : %d\n", client_addr.sin_port);
                      //printf("server port : %d\n",list_data_server[h].addr.sin_port);
                      printf("buff = %s\n",buff);
                      client_addr = list_data_server[h].addr;
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
                      printf("%s\n",buff);
                      envoi_ok++;
                      printf("envoi_ok %d\n",envoi_ok);
                    }
                  }
                  printf("sc = %d\n",serveur_connecte);
                  if(serveur_connecte == 0)
                  {
                    client_addr = list_table_user[i].addr;
                    snprintf(buff,N,"Le serveur que vous avez indiquer est indisponible\n");
                    if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                    {
                      perror("erreur sockett sendefrrqz");
                      exit(EXIT_FAILURE);
                    }
                    break;
                  }
                  if(serveur_connecte == 1 && envoi_ok == count_type)
                  {
                    printf("amir\n");
                    client_addr = list_table_user[i].addr;
                    if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                    {
                      perror("erreur sockett sendefrrqz");
                      exit(EXIT_FAILURE);
                    }
                    break;
                  }
                }
              }
              }
              if(test == 0)
              {
                snprintf(buff,N,"Vous n'avez pas le droit pour ecrire ce champ\n");
                if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                {
                  perror("erreur sockett sendefrrqz");
                  exit(EXIT_FAILURE);
                }
              }
            }
          }
        }
      }
    }
    else if(strncmp("supprimer",strToken,9) == 0)
    {
      //char type[32]={0};
      //strToken = strtok(NULL," \n");
      //printf("supp strtok = %s\n",strToken);
      //strcpy(type,strToken);
      for(int j = 0; j < nb_user_connect; j++)
      {
        if(client_addr.sin_port == list_table_user[j].addr.sin_port)
        {
        // a l'aide du port on trouve l'uilistaeur et on regarde maintenan si le login est dans le tableau qui a stock les login
        for(unsigned int k = 0; k < tab_user->nb_utilisateurs; k++)
        {
          if(strcmp(list_table_user[j].login,tab_user->table[k].login) == 0)
          {
            //on parcourt la liste des attributs de l'user pour voir si il a le champ dans sa liste des attributs
            int test = 0;
            for(unsigned int h = 0; h < tab_user->table[j].taille_attributs; h++)
            {
              //if(strcmp(type,tab_user->table[j].attribut[h]) == 0)
              //{
                test = 1;
                int serveur_connecte = 0;
                int droit = 0;
                int count_type = 0;
                int envoi_ok = 0;
                for(int i =0; i < nombre_server; i++)
                {
                  if(strcmp(list_data_server[i].type,tab_user->table[j].attribut[h]) == 0)
                  {
                    count_type++;
                    serveur_connecte = 1;
                    if(droit == 0)
                    {
                      snprintf(buff,N,"OK vous avez le droit de supprimer\n");
                      if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                      {
                        perror("erreur sockett sendefrrqz");
                        exit(EXIT_FAILURE);
                        }
                      }
                      droit =1;
                      snprintf(buff,N+2048,"supprimer %s\n",tab_user->table[k].login);
                      printf("buff = %s\n",buff);
                      client_addr = list_data_server[i].addr;
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
                      printf("%s\n",buff);
                      envoi_ok++;
                    }
                  }
                  if(serveur_connecte == 1 && envoi_ok == count_type)
                  {
                    client_addr = list_table_user[j].addr;
                    strToken = strtok(buff," ");
                    int supp_send = 0;
                    if(strcmp("Rien",strToken) == 0)
                    {
                      supp_send =1;
                      snprintf(buff,N,"Il n'y a rien à supprimer dans ce champ\n");
                      if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                      {
                        perror("erreur sockett sendefrrqz");
                        exit(EXIT_FAILURE);
                      }
                    }
                    if(supp_send == 0)
                    {
                      snprintf(buff,N,"La suppression est un succès\n");
                      if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                      {
                        perror("erreur sockett sendefrrqz");
                        exit(EXIT_FAILURE);
                      }
                    }
                  }
                  if(serveur_connecte == 0)
                  {
                    snprintf(buff,N,"Le serveur que vous essayez de joindre n'est pas connecté\n");
                    if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                    {
                      perror("erreur sockett sendefrrqz");
                      exit(EXIT_FAILURE);
                    }
                  }
                //}
                if(test == 0)
                {
                snprintf(buff,N,"Vous n'avez pas les droits pour accéder à ce champ\n");
                if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
                {
                  perror("erreur sockett sendefrrqz");
                  exit(EXIT_FAILURE);
                }
              }
            }
          }
        }
      }
    }
  }
  else if(strncmp("aide",strToken,4) == 0)
  {
    snprintf(buff,N,"**Voici les syntaxes à utiliser**\n");
    strcat(buff,"\n");
    strcat(buff,"Pour lire un ou plusieurs champs:\n");
    strcat(buff,"lire champ1 champ2\n");
    strcat(buff,"Pour ecrire un ou plusieurs champs:\n");
    strcat(buff,"ecrire champ1:donnee1 champ2:donnee2\n");
    strcat(buff,"Pour supprimer un ou plusieurs champs:\n");
    strcat(buff,"supprimer champ1 champ2\n");
    strcat(buff,"Pour se deconnecter :\n");
    strcat(buff,"bye\n");
    if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
    {
      perror("erreur sockett sendefrrqz");
      exit(EXIT_FAILURE);
    }
  }
  else if(strcmp("clear",strToken) == 0)
  {
    snprintf(buff,N,"\n");
    for(int i = 0; i < 20; i++)
    {
      strcat(buff,"\n");
    }
    if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
    {
      perror("erreur sockett sendefrrqz");
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    snprintf(buff,N,"La commande que vous avez entré n'existe pas\n");
    if( (sendto(sock_fd,buff,N,0,(struct sockaddr *)&client_addr, sizeof(client_addr))) == -1)
    {
      perror("erreur sockett sendefrrqz");
      exit(EXIT_FAILURE);
    }
  }
}

printf("***** Serveur hors ligne *****\n\n");

return 0;
}
