#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/shm.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <signal.h>

#include <stdlib.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <list>
#include <string>
#include <iostream>
#include <cstring>
using namespace std;

#define SA struct sockaddr
#define BUFSIZE      1024
#define MAXHOSTNAME    80
#define MESSAGE_SIZE   80
#define SIZE           10
#define USER_SIZE      25
#define ADDR_SIZE      16
#define SERVPANEL       1
#define SERVAUDIENCE    2
#define GETUSERNAME     3
#define CTRLC           4
#define DISCONNECTED    5
#define SHOWISOVER      6




 
 

#endif // HEADER_H_INCLUDED
