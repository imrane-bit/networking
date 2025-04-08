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
#define CHAT_FILE "chat.txt"
#define GETINPUTSTR "saysomething>>"


/*this handles a special signal 
 * means the server is exiting*/
void signal_handler(int signl)
{
  FILE *chat_file;
  char *exitmsg = "server exited , terminating...";
  if(SIGUSR1 == signl)
  {
    chat_file = fopen(CHAT_FILE,"w");
    fclose(chat_file);
    printf("%s\n",exitmsg);
    exit(0);
  }
}

/*those are functions used to display the 
 * chat to the user and keep the ui nice ,
 * i don't think they are robust*/
void print_chat(char *filename)
{
  FILE *chat_file = fopen(CHAT_FILE,"r");
  char line[BUFFSIZE];
  while (0 < fgets(line,BUFFSIZE-1,chat_file))
  {
    printf("%s",line);
  }
  fclose(chat_file);
}

void clear_chat(void)
{
  fputs("\033c", stdout);
}

void update_chat(char *filename)
{
  clear_chat();
  print_chat(filename);
}


/*listen for any new messages and updated the 
 * chat file and the displayed chat if there is any*/
void *listening_funtion(void *serversocket)
{

  /*yes , this just uses a file*/
  FILE *chat_file = fopen(CHAT_FILE,"r");
  int *socketfd = (int*) serversocket;
  char message[BUFFSIZE + 100];

  while(1)
  {
    chat_file = fopen(CHAT_FILE,"a");
    memset(message,0,BUFFSIZE);
    if (0 >= read(*socketfd,message,BUFFSIZE-1))
    {
      perror("did you hear anything?");
      return NULL;
    }
    fputs(message, chat_file);
    fclose(chat_file);
    update_chat(CHAT_FILE);
    printf(GETINPUTSTR);
    fflush(stdout);
  }

}
