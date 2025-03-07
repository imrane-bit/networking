#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#define BUFFSIZE 4096
#define SERVER_PORT 6342
void signal_handler(int signl)
{
  char *exitmsg = "server exited , terminating...";
  if(SIGUSR1 == signl)
  {
    printf("%s\n",exitmsg);
    exit(0);
  }
}
void *listening_funtion(void *serversocket)
{
  int *socketfd = (int*) serversocket;
  char message[BUFFSIZE];
  while(1)
  {
    memset(message,0,BUFFSIZE);
    if (0 >= read(*socketfd,message,BUFFSIZE-1))
    {
      perror("did you hear anything?");
      return NULL;
    }
    printf("%s",message);
  }

}
int main(int argc,char *argv[])
{
  signal(SIGUSR1,signal_handler);
  #define SERVER_ADDRESS argv[1]
  pthread_t recv_thread;
  struct sockaddr_in address;
  char message[BUFFSIZE], *servername = "localhost";
  char *client_exit_sig = "cutmeoff";
  int opt = 1;
  pid_t self_pid = getpid();
  char response[BUFFSIZE];
  int socketfd= socket(AF_INET,SOCK_STREAM,0);
  if(socketfd < 0)
  {
    perror("the system said no sockets :(");
    return errno;
  }
  address.sin_family = AF_INET;
  address.sin_port = htons(SERVER_PORT);
  if (inet_pton(AF_INET, SERVER_ADDRESS, &address.sin_addr)
    <= 0) {
    printf(
      "\nInvalid address/ Address not supported \n");
    return -1;
  }
  setsockopt(socketfd, SOL_SOCKET,
             SO_REUSEADDR, &opt,
             sizeof(opt));
  if (0 != connect(socketfd,
                   (struct sockaddr*)&address,
                   sizeof(address)))
  {
    perror("looks like connecting to a socket is that hard.\n");
    return errno;
  }

  int serversocket;
  serversocket = socketfd;
  pthread_create(&recv_thread,NULL,listening_funtion,(void*) &serversocket);
    if(0 >= send(socketfd,&self_pid,sizeof(pid_t),MSG_EOR))
    {
      perror("sorry cannot speak");
      return errno;
    }
  while (1) {
    if (0 >= fgets(response,BUFFSIZE-1,stdin)) {
      perror("no input?");
      return errno;
    }
    if(0 >= send(socketfd,response,BUFFSIZE-1,MSG_EOR))
    {
      perror("sorry cannot speak");
      return errno;
    }
    if(!strncmp(response,client_exit_sig,strlen(client_exit_sig)))
      break;

  }
  close(socketfd);
  return errno;
}
