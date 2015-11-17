#include "def.h"
#define MAXHOSTNAME 80
#define AUDIENCE 0
#define ACTOR 1
#define BUFSIZE 1024
void exit (int);
char buf[BUFSIZE];
char rbuf[BUFSIZE];
void GetUserInput ();
void ignoreCTRD (int sig);
void ignoreCTRC (int sig);
int rc, cc;
int servsock, clisock;
char *username;


main (argc, argv)
int argc;
char *argv[];
{
    int childpid;
    struct sockaddr_in remoteServer;
    struct sockaddr_in localServer;
    struct sockaddr_in client;
    struct hostent *hp, *gethostbyname ();
    struct servent *sp;
    struct sockaddr_in from;
    struct sockaddr_in addr;
    int fromlen;
    int length;
    int MODE;
    int ServerPort;
    int Port;

    if (argc == 1) {
        printf (" Usage: panclient <host> <port> \n");
        exit (0);
    }
	//ignore ctrl+C
	signal (SIGINT, SIG_IGN);
	//ignore quit signal
	signal (SIGQUIT, SIG_IGN);

    Port = atoi(argv[2]);
   
        printf ("CLIENT is a panalist\n");
        printf ("Server Port is: %d\n", Port);


    if ((hp = gethostbyname (argv[1])) == NULL) {
        addr.sin_addr.s_addr = inet_addr (argv[1]);
        if ((hp = gethostbyaddr ((char *) &addr.sin_addr.s_addr,
                                 sizeof (addr.sin_addr.s_addr), AF_INET)) == NULL) {
            fprintf (stderr, "Can't find host %s\n", argv[1]);
            exit (-1);
        }
    }
    bcopy (hp->h_addr, &(remoteServer.sin_addr), hp->h_length);
    remoteServer.sin_family = AF_INET;
    remoteServer.sin_port = htons(Port);
    clisock = socket (AF_INET, SOCK_STREAM, 0);
    if (connect (clisock, (SA *) & remoteServer, sizeof (remoteServer)) < 0) {
        close (clisock);
        perror ("connecting stream socket");
        exit (0);
    }

    if ((username = getlogin()) == NULL) perror("Who are you?\n");

    if ( send(clisock, username, strlen(username), 0) < 0)
        perror("send username  message");
   
	childpid = fork ();
	if (childpid == 0) {
		
		signal (SIGINT, ignoreCTRC);
		signal (SIGQUIT, ignoreCTRD);
		
		GetUserInput ();
	}
   
    for (;;) {
        cleanup (rbuf);
        if ((rc = recv (clisock, rbuf, sizeof (buf), 0)) < 0) {
            perror ("receiving stream  message");
            exit (-1);
        }
        if (rc > 0) {
            rbuf[rc] = '\0';
            if (strcmp(rbuf," \b") != 0)
                printf ("<: %s\n", rbuf);
            // printf (":> ");
            fflush (stdout);
        }
        else {
            printf ("Disconnected..\n");
            close (clisock);
            exit (0);
        }
    }
}

void ignoreCTRC (int sig)
{
    signal(SIGINT, ignoreCTRC);

   printf("Use Ctr+D to exit ...\n");
}

void ignoreCTRD (int sig)
{
    signal(SIGQUIT, ignoreCTRD);

   printf("Use Ctr+D to exit ...\n");
}

cleanup (buf)
char *buf;
{
    int i;
    for (i = 0; i < BUFSIZE; i++) buf[i] = '\0';
}

void GetUserInput ()
{
    for (;;) {
        //printf (":> ");
        fflush (stdout);
        cleanup (buf);
        rc = read (0, buf, sizeof (buf));
        if (rc == 0)
            break;
        if (send (clisock, buf, rc, 0) < 0)
            perror ("sending stream message");
    }
    printf ("Disconnecting ... \n");
    close (clisock);
    kill (getppid (), 9);
    exit (0);
}
