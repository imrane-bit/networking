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


void signal_handler(int signl)
{
  char *exitmsg = "server exited , terminating...";
  if(SIGUSR1 == signl)
  {
    printf("%s\n",exitmsg);
    exit(0);
  }
}
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
    update_chat(CHAT_FILE);
    printf(GETINPUTSTR);
    fflush(stdout);
  }

}
