#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define N 1024

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
