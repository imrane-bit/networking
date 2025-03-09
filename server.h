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
#define ADDRESS "192.168.1.59"
#define server_exit_sig "serverexit\n"
#define CHAT_FILE "chat.txt"

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
  SETSOCK_ERROR= -10,
  SOCKET_ERROR = -11
};
enum VALUES 
{
  MAX_CLIENTS = 10,
  PORT = 6342,
  BUFFSIZE = 4096

};
enum FLAGS
{ 
  SOCKET_NOT_IN_USE = -100
};

extern int clients[MAX_CLIENTS];
extern pid_t clients_pid[MAX_CLIENTS];
extern pthread_t threads[MAX_CLIENTS];

int socketfd;

int broadcast(char * message,int clientnumber);
void * handle_client(void* clientnumber_ptr);
void * server_interface(void *notused);



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
    if(SOCKET_NOT_IN_USE == clients[i] 
      || clientnumber == i)
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
  FILE *chat_file = fopen(CHAT_FILE,"r");
  int clientnumber = *((int*) clientnumber_ptr);
  int clientsocket = clients[clientnumber];
  char message[BUFFSIZE];
  char printed_message[BUFFSIZE + 100];
  char *exit_sig = "exit\n";
  char *client_exit_sig = "cutmeoff\n";
  while (1) 
  {
    chat_file = fopen(CHAT_FILE,"a");
    if (0 >= read(clientsocket,message,BUFFSIZE-1))
    {
      perror("did you hear anything?");
      goto error_handle;
    }
    sprintf(printed_message,
            "client %d)>> %s",
            clientnumber,message);
    fputs(printed_message, chat_file);
    fclose(chat_file);
    broadcast(message,clientnumber);
    if(!strncmp(message,
                client_exit_sig,
                strlen(client_exit_sig)))
    {
      close(clientsocket);
      clientsocket = SOCKET_NOT_IN_USE;
      clients[clientnumber]= SOCKET_NOT_IN_USE;
      goto error_handle;
    }
  }
error_handle:
  if(SOCKET_NOT_IN_USE != clientsocket)
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
      FILE *chat_file = fopen(CHAT_FILE,"w");
      fclose(chat_file);
      for(int i =0 ; i < MAX_CLIENTS ; i++)
      {
        if (SOCKET_NOT_IN_USE == clients[i])
          continue;
        kill(clients_pid[i],SIGUSR1);
        close(clients[i]);
      }
      close(socketfd);
      exit(0);
    }
  }
}
