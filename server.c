#include "server.h"


/*array holding clients sockets*/
int clients[MAX_CLIENTS];
/*array that has the pid of each client*/
pid_t clients_pid[MAX_CLIENTS];
/*array that has the thread id of each client*/
pthread_t threads[MAX_CLIENTS];
/*the server socket*/
int socketfd;

int main()
{
  struct sockaddr_in address;
  client_info information;
  int opt = 1;
  
  /*the thread id of the thread that lets 
   * the operator interface with the server*/
  pthread_t interface_thread;
  
  /*initializing , each client will increment this*/
  int clientnumber=0 ;

  socketfd= socket(AF_INET,SOCK_STREAM,0);

  if(socketfd < 0)
  {
    perror("the system said no sockets :(");
    errno = SOCKET_ERROR;
    goto error_handle;
  }

  address.sin_family = AF_INET;
  address.sin_port = htons(PORT);

  /*making the socket reusable i guess , 
   * forgot what old me was doing 
   * comment right away kids*/
  if ( 0 != setsockopt(socketfd, SOL_SOCKET,
             SO_REUSEADDR, &opt,
             sizeof(opt)))
  {
    errno = SETSOCK_ERROR;
    goto error_handle;
  }




  /*settin up the address*/
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

  /*starting the interface of the server 
   * it is now mainly used for clean exit*/

  pthread_create(&interface_thread,
                 NULL,
                 server_interface,
                 (void*) NULL);



  /*initializing the clients sockets*/
  for(int i = 0;i<MAX_CLIENTS;i++)
  {
    clients[i] = SOCKET_NOT_IN_USE;
  }




  /*main loop*/
  for(int i=0;i<MAX_CLIENTS;i++) {

    /*each client is known by his number*/
    information.client_number = i;

    /*waiting for clients*/

    if (0 != listen(socketfd,1))
    {
      perror("no ,i am not listening\n");
      goto error_handle;
    }


    /*establishing connetctions*/
    if( (clients[i] = accept(socketfd,NULL,NULL)) < 0)
    {
      perror("no , not accepted , I am sorry :b");
      goto error_handle;
    }



    /*the client sends his thread id , i know this is not 
     * secure, relying on valid input is bad */
    if (0 >= read(clients[i],clients_pid+i,sizeof(pid_t)))
    {
      goto error_handle;
    }
    clientnumber =i;

    
    /*creating a handler thread for the 
     * client and going on to the next*/
    pthread_create(threads+i,NULL,handle_client,(void*) &clientnumber);
  }
error_handle:
  close(socketfd);
  exit(0);
}

