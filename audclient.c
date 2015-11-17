#include "def.h"
#define MAXHOSTNAME 80
#define AUDIENCE 0
#define ACTOR 1
#define BUFSIZE 1024
void exit (int);
char buf[BUFSIZE];
char rbuf[BUFSIZE];
void GetUserInput ();
void sig_leaving();
int rc, cc;
int servsock, clisock;
char *username;

void ignoreQUIT (int sig);
void ignoreCTRD (int sig);

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
        printf (" Usage: audclient <host> <port> \n");
        exit (0);
    }
	
	//ignore quit signal
	signal (SIGQUIT, SIG_IGN);


    Port = atoi(argv[2]);
	Port++;
    printf ("CLIENT is an audience\n");
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
		
		signal (SIGQUIT, ignoreQUIT);	
		signal (SIGINT,sig_leaving );
		GetUserInput ();
	}
   
    for (;;) {
		signal (SIGQUIT, SIG_IGN);
		signal (SIGINT,SIG_IGN);
        cleanup (rbuf);
        if ((rc = recv (clisock, rbuf, sizeof (buf), 0)) < 0) {
            perror ("receiving stream  message");
            exit (-1);
        }
        if (rc > 0) {
            rbuf[rc] = '\0';
            if (strcmp(rbuf," \b") != 0)
                printf ("%s\n", rbuf);
            
            fflush (stdout);
        }
        else {
            printf ("Disconnected..\n");
            close (clisock);
            exit (0);
        }
    }
}

cleanup (buf)
char *buf;
{
    int i;
    for (i = 0; i < BUFSIZE; i++) buf[i] = '\0';
}
void sig_leaving()
{
	printf ("\n\nI am leaving ... \n");
    close (clisock);
    kill (getppid (), 9);
    exit (0);
}

void ignoreCTRD (int sig)
{
   signal(SIGINT, ignoreCTRD);
   printf("\nUse Ctr+C to exit ...\n");
}

void ignoreQUIT (int sig)
{
    signal(SIGQUIT, ignoreQUIT);

   printf("\nUse Ctr+C to exit ...\n");
}

void GetUserInput ()
{
    for (;;) {
        //printf ("Received: ");
        fflush (stdout);
        cleanup (buf);
        rc = read (0, buf, sizeof (buf));
        if (rc == 0)
            break;
        if (send (clisock, buf, rc, 0) < 0)
            perror ("sending stream message");
    }
	
	
	int pid1 = fork ();
	if (pid1==0)
	{
		signal (SIGQUIT, SIG_IGN);
		signal (SIGINT,SIG_IGN );
		for (;;)
		{
		signal(SIGQUIT,ignoreCTRD);
		}
		exit(0);
	}
	wait(NULL);
    printf ("EOF... exit\n");
    close (clisock);
    kill (getppid (), 9);
    exit (0);
}
