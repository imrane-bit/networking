#include "server.h"
int clients[MAX_CLIENTS];
pid_t clients_pid[MAX_CLIENTS];
pthread_t threads[MAX_CLIENTS];
int broadcast(char * message,int clientnumber);
void * handle_client(void* clientnumber_ptr);
void * server_interface(void *notused);
int socketfd;

int main()
{
  struct sockaddr_in address;
  client_info information;
  int opt = 1;
  pthread_t interface_thread;
  int clientnumber=0 ;
  socketfd= socket(AF_INET,SOCK_STREAM,0);
  pthread_create(&interface_thread,NULL,server_interface,(void*) NULL);
  for(int i = 0;i<MAX_CLIENTS;i++)
  {
    clients[i] = -1;
  }
  if(socketfd < 0)
  {
    perror("the system said no sockets :(");
    errno = SOCKET_ERROR;
    goto error_handle;
  }
  if ( 0 != setsockopt(socketfd, SOL_SOCKET,
             SO_REUSEADDR, &opt,
             sizeof(opt)))
  {
    errno = SETSOCK_ERROR;
    goto error_handle;
  }
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

    if (0 >= read(clients[i],clients_pid+i,sizeof(pid_t)))
    {
      goto error_handle;
    }
    clientnumber =i;
    
  pthread_create(threads+i,NULL,handle_client,(void*) &clientnumber);
}
error_handle:
close(socketfd);
exit(0);
}

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

void * server_interface(void *notused)
{
  char command[BUFFSIZE];
  while(1)
  {
    memset(command,0,BUFFSIZE);
    if (0 >= fgets(command,BUFFSIZE-1,stdin))
    {
      perror("cannot read from stdin");
      return NULL;
    }
    if(!strncmp(command,
                server_exit_sig,
                strlen(server_exit_sig)))
    {
      for(int i =0 ; i < MAX_CLIENTS ; i++)
      {
        if (-1 == clients[i])
          continue;
        kill(clients_pid[i],SIGUSR1);
        close(clients[i]);
      }
      close(socketfd);
      exit(0);
    }
  }
}
