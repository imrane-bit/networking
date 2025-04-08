#include "client.h"

int main(int argc,char *argv[])
{
  #define SERVER_ADDRESS argv[1]
  
  /*to handle kill and int signals*/
  signal(SIGUSR1,signal_handler);

  int serversocket;
  /*thread id for the thread that receives and displays msgs*/
  pthread_t recv_thread;
  struct sockaddr_in address;

  char  response[BUFFSIZE],
  message[BUFFSIZE],
  *servername = "localhost";
  
  
  /*a special input that shuts down the client*/
  char *client_exit_sig = "cutmeoff";
  int opt = 1;
  
  /*getting this clients pid to provide it for the server*/
  pid_t self_pid = getpid();


  /*this server socket*/
  int socketfd= socket(AF_INET,SOCK_STREAM,0);

  if(socketfd < 0)
  {
    perror("the system said no sockets :(");
    return errno;
  }
  
  /*setting up the address*/
  address.sin_family = AF_INET;
  address.sin_port = htons(SERVER_PORT);
  if (inet_pton(AF_INET, SERVER_ADDRESS, &address.sin_addr)
    <= 0) {
    printf(
      "\nInvalid address/ Address not supported \n");
    return -1;
  }

  /*setting up the socket to be reusable*/
  setsockopt(socketfd, SOL_SOCKET,
             SO_REUSEADDR, &opt,
             sizeof(opt));

  /*connet to server*/
  if (0 != connect(socketfd,
                   (struct sockaddr*)&address,
                   sizeof(address)))
  {
    perror("looks like connecting to a socket is that hard.\n");
    return errno;
  }

  /*get rid of anything on the screen*/
  clear_chat();
  printf("welcome to the chat\n");

  serversocket = socketfd;


  /*start the thread of the fucntion that listens to anything 
   * from the server and displays it */
  pthread_create(&recv_thread,NULL,listening_funtion,(void*) &serversocket);

  /*send the pid of this client to the server for 
   * management */
    if(0 >= send(socketfd,&self_pid,sizeof(pid_t),MSG_EOR))
    {
      perror("sorry cannot speak");
      return errno;
    }

  /*prompt*/
  printf("%s",GETINPUTSTR);
  fflush(stdout);

  /*main loop , transmits user messages to the server*/
  while (1) {

    if (0 >= fgets(response,BUFFSIZE-1,stdin)) {
      perror("no input?");
      return errno;
    }

    printf(GETINPUTSTR);
    fflush(stdout);
    if(0 >= send(socketfd,response,BUFFSIZE-1,MSG_EOR))
    {
      perror("could not send response");
      return errno;
    }

    /*check for the special exit command */
    if(!strncmp(response,client_exit_sig,strlen(client_exit_sig)))
      break;

  }
  close(socketfd);
  return errno;
}

