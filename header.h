#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <arpa/inet.h>

#include <stdint.h>  //uint16_t

#define MAX 1024
#define QUEUE_MAX 5   // max waiting clients
#define DB_SIZE 3

typedef struct User
{
	char* name;
	char* id;
	char* password;
} User;

int recv_all(int sockfd, char* buf, size_t len, int flags);
int send_all(int sockfd, const char* buf, size_t len, int flags);

