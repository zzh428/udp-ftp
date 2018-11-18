#ifndef INC_H
#define INC_H

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <net/if.h> 
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ifaddrs.h>
#include <regex.h>
#include <pthread.h>

int transferState = 0;
struct retr_args
{
	int connfd;
	int fd;
	char * args;
	int rest;
};
struct stor_args
{
	int connfd;
	int fd;
	char * args;
	char * cmd;
};

#endif