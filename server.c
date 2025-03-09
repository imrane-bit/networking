#include "server.h"


/*array holding sockets*/
int clients[MAX_CLIENTS];
/*array that has the pids of each client*/
pid_t clients_pid[MAX_CLIENTS];
pthread_t threads[MAX_CLIENTS];
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
    clients[i] = SOCKET_NOT_IN_USE;
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

