#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#define ADDRESS "192.168.1.59"

typedef struct clientinfo {
  int client_number;
} client_info;

enum ERRORS 
{
  SEND_ERROR =-5,
  RECV_ERROR =-6,
  READ_ERROR =-7,
  ACCEPT_ERROR = -8,
  BIND_ERROR = -9,
  ACCEPT_ERROR= -10,
  SOCKET_ERROR = -11
};
enum VALUES 
{
  MAX_CLIENTS = 10,
  PORT = 6342,
  BUFFSIZE = 4096

};
int clients[MAX_CLIENTS];
pthread_t threads[MAX_CLIENTS];

int broadcast(char * message,int clientnumber)
{
  char indicator[100];
  snprintf(indicator,99,
          "(client %d)>>",
          clientnumber);
  char broadcast_text[BUFFSIZE+100];
  snprintf(broadcast_text,BUFFSIZE+99,
          "(client %d)>>%s",clientnumber,
          message);
  for (int i = 0;i<MAX_CLIENTS;i++)
  {
    if(-1 == clients[i] || clientnumber == i)
      continue;

    if(0 >= send(clients[i],
                 broadcast_text,
                 strlen(broadcast_text),
                 MSG_EOR))
    {
      perror("i forgot how to shout");
      return SEND_ERROR;
    }
  }
}
void * handle_client(void* clientnumber_ptr)
{
  int clientnumber = *((int*) clientnumber_ptr);
  int clientsocket = clients[clientnumber];
  char message[BUFFSIZE];
  char *exit_sig = "exit\n";
  char *client_exit_sig = "cutmeoff\n";
  while (1) {
    if (0 >= read(clientsocket,message,BUFFSIZE-1))
    {
      perror("did you hear anything?");
      goto error_handle;
    }
    broadcast(message,clientnumber);
    printf("client %d)>> %s",clientnumber,message);
    if(!strncmp(message,
                client_exit_sig,
                strlen(client_exit_sig)))
    {
      close(clientsocket);
      clientsocket = -1;
      clients[clientnumber]= -1;
      goto error_handle;
    }
  }
error_handle:
  if(-1 != clientsocket)
    close(clientsocket);
  return NULL;
}
int main()
{
  struct sockaddr_in address;
  client_info information;
  int opt = 1;
  int clientnumber=0;
  int socketfd= socket(AF_INET,SOCK_STREAM,0);
  for(int i = 0;i<MAX_CLIENTS;i++)
  {
    clients[i] = -1;
  }
  if(socketfd < 0)
  {
    perror("the system said no sockets :(");
    goto error_handle;
    errno = SOCKET_ERROR;
  }
  setsockopt(socketfd, SOL_SOCKET,
             SO_REUSEADDR, &opt,
             sizeof(opt));
  address.sin_family = AF_INET;
  address.sin_port = htons(PORT);

  if (inet_pton(AF_INET, ADDRESS, &address.sin_addr)
    <= 0) 
 {
    printf(
      "\nInvalid address/ Address not supported \n");
  goto error_handle;
}
if (0 != bind(socketfd,
              (struct sockaddr*)&address,
              sizeof(address)))
{
  perror("looks like binding a socket is that hard.\n");
  goto error_handle;
}
for(int i=0;i<MAX_CLIENTS;i++) {
  information.client_number = i;
  if (0 != listen(socketfd,1))
  {
    perror("no one ever wants to actually listen :D");
    goto error_handle;
  }
  if( (clients[i] = accept(socketfd,NULL,NULL)) < 0)
  {
    perror("no , not accepted , I am sorry :b");
    goto error_handle;
  }

    clientnumber =i;
  pthread_create(&threads[i],NULL,handle_client,(void*) &clientnumber);


}
error_handle:
close(socketfd);
exit(0);
}
