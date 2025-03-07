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

