#define _XOPEN_SOURCE   500
#define __EXTENSIONS__
#define u_char char

#define SA struct sockaddr
#define  h_addr h_addr_list[0]

#ifndef MULTICAST_H
#define MULTICAST_H

/* include files */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <pthread.h>
#include <pwd.h>
#include <regex.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>
#include <sys/shm.h>


/* define statements */
#define MULTICAST_ADDRESS_BASE  "224.0.0."
#define MULTICAST_PORT_BASE  10000
#define DEFAULT_MULTICAST_TTL_VALUE  2
        /* 2 limits the scope to only one router */

#ifndef	INADDR_NONE
#define	INADDR_NONE	0xffffffff
#endif	/* INADDR_NONE */

#define NAME_LEN 100
#define MAX_LEN 1024

#define TRUE 1
#define FALSE 0

/* functions to support the essential multicasting functions */
extern void leaveGroup(int recvSock, char *group);
extern void joinGroup(int s, char *group);
extern void reusePort(int sock);
extern void displayDaddr(int sock);
extern void setTTLvalue(int s,u_char *i);
extern void setLoopback(int s,u_char loop);

/* external variables */

extern int	errno;
extern char	*sys_errlist[];


#endif /* MULTICAST_H */

struct in_pktinfo {
    unsigned int   ipi_ifindex;  /* Interface index */
    struct in_addr ipi_spec_dst; /* Local address */
    struct in_addr ipi_addr;     /* Header Destination
                                    address */
};


struct ip_mreq 
{
        struct in_addr imr_multiaddr;   /* IP multicast address of group */
        struct in_addr imr_interface;   /* local IP address of interface */
};

