#include "def.h"
void exit (int);

#define SA struct sockaddr
#define MAXLINE 1024

#define MAXHOSTNAME 80
#define URGENT 1
#define NORMAL 0
void reusePort (int sock);
void PIPEhandle (int sig);
void CHLDhandle (int sig);
void QUIThandle (int sig);
void INThandle (int sig);
void USR1handle (int sig);
void USR2handle (int sig);

int ACTsd, AUDsd, psd, sd;
struct sockaddr_in ACTserver;
struct sockaddr_in AUDserver;
struct hostent *hp, *gethostbyname ();
struct servent *sp;
struct sockaddr_in from;
int fromlen;
int length;
char ThisHost[80];
int childpid;
char buf[512];
int MODE = URGENT;
char *byby="Show is Over\n";

int i;
int myindex;
char *clientName ;
char *clientIP ;
int clientPort ;

#define MAXACT 10
#define MAXAUD 10

void shmInit();
void MemAttach();
void ACTServ();
void AUDServ ();
void ACTHand();
void RefreshLists();
int ACTpid, AUDpid, ACTHandpid;
int ACTgetfd ();
int AUDgetfd ();


int  ACTsegment_id1,  ACTsegment_id2,  ACTsegment_id3;
int  AUDsegment_id1,  AUDsegment_id2,  AUDsegment_id3;
int  MSGsegment;
int  PIPEflagSegment;

int *ACTconnfd;
struct sockaddr_in *ACTList;

struct user_name {
    char    logname[10];
};

char *MSGbuffer;
int  *PIPEflag;

struct user_name *ACTname;
struct user_name *AUDname;

int *AUDconnfd;
struct sockaddr_in *AUDList;
char username[MAXLINE];



main (argc, argv)
int argc;
char *argv[];
{

    if (argc != 2) {
        printf (" Usage: tserver <port> \n");
        exit (0);
    }

    signal (SIGPIPE, PIPEhandle);
    signal (SIGCHLD, CHLDhandle);
    signal (SIGQUIT, QUIThandle);
    signal (SIGINT, INThandle);
    signal (SIGUSR1, USR1handle);
    shmInit();


    ACTserver.sin_family = AF_INET;
    ACTserver.sin_addr.s_addr = htonl (INADDR_ANY);
    ACTserver.sin_port = htons (atoi (argv[1]));

    AUDserver.sin_family = AF_INET;
    AUDserver.sin_addr.s_addr = htonl (INADDR_ANY);
    AUDserver.sin_port = htons (atoi (argv[1]) +1);

    ACTsd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    reusePort (ACTsd);

    if (bind (ACTsd, (SA *) & ACTserver, sizeof (ACTserver)) < 0)
    {
        close (ACTsd);
        perror ("binding name to stream socket");
        exit (-1);
    }

    length = sizeof (ACTserver);
    if (getsockname (ACTsd, (SA *) & ACTserver, &length))
    {
        perror ("getting socket name");
        exit (0);
    }

    listen (ACTsd, 4);

    AUDsd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    reusePort (AUDsd);

    if (bind (AUDsd, (SA *) & AUDserver, sizeof (AUDserver)) < 0)
    {
        close (AUDsd);
        perror ("binding name to stream socket");
        exit (-1);
    }

    length = sizeof (AUDserver);
    if (getsockname (AUDsd, (SA *) & AUDserver, &length))
    {
        perror ("getting socket name");
        exit (0);
    }

    listen (AUDsd, 4);

    AUDpid = fork ();
    if (AUDpid == 0)
    {
        //MemAttach();
        signal (SIGINT, SIG_IGN);
        signal (SIGQUIT, SIG_IGN);
        signal (SIGUSR2, USR2handle);
        AUDServ ();
    }
	
    printf ("Forked AUDIENCE Server, Port = (%d), PID = %d\n", ntohs (AUDserver.sin_port), AUDpid);


    printf ("Started ACTORS Server,  Port = (%d)\n", ntohs (ACTserver.sin_port));
    ACTServ ();
}

void INThandle (int sig)
{
    signal (SIGINT, INThandle);
    RefreshLists();
    usleep(100);
    RefreshLists();
    usleep(100);

    //printf("INThandle\n");
    printf("\n\n");

    printf("\nSTATUS:\n");
    printf("-------\n");
    printf("\nACTORS:\n");
    for (i = 0; i < MAXACT; i++)
        if (ACTconnfd[i] != -1) {
            printf("(%s:%d) [%s]\n",
                   inet_ntoa(ACTList[i].sin_addr), ntohs(ACTList[i].sin_port), ACTname[i].logname);
        }

    printf("\nAUDIENCE:\n");
    for (i = 0; i < MAXACT; i++)
        if (AUDconnfd[i] != -1) {
            printf("(%s:%d) [%s]\n",
                   inet_ntoa(AUDList[i].sin_addr), ntohs(AUDList[i].sin_port), AUDname[i].logname);
        }
    printf("--------------------------------------------------\n\n");

}

void QUIThandle (int sig)
{
    signal (SIGQUIT, QUIThandle);
    printf("QUIThandle\n");
    sprintf(MSGbuffer,"%s", byby);
    kill (AUDpid, SIGUSR2);
    kill (getpid(), SIGUSR1);
    sleep(2);
    system("pkill -9 tserver");
    exit(0);
}

void shmInit()
{

    ACTsegment_id1 = shmget (IPC_PRIVATE, MAXACT * sizeof (int), S_IRUSR | S_IWUSR);
    ACTsegment_id2 = shmget (IPC_PRIVATE, MAXACT * sizeof (struct sockaddr_in), S_IRUSR | S_IWUSR);
    ACTsegment_id3 = shmget (IPC_PRIVATE, MAXACT * sizeof (struct user_name), S_IRUSR | S_IWUSR);

    AUDsegment_id1 = shmget (IPC_PRIVATE, MAXAUD * sizeof (int), S_IRUSR | S_IWUSR);
    AUDsegment_id2 = shmget (IPC_PRIVATE, MAXAUD * sizeof (struct sockaddr_in), S_IRUSR | S_IWUSR);
    AUDsegment_id3 = shmget (IPC_PRIVATE, MAXAUD * sizeof (struct user_name), S_IRUSR | S_IWUSR);

    MSGsegment = shmget (IPC_PRIVATE, MAXLINE*sizeof(char), S_IRUSR | S_IWUSR);
    PIPEflagSegment = shmget (IPC_PRIVATE, sizeof(int), S_IRUSR | S_IWUSR);

    ACTconnfd = (int *) shmat (ACTsegment_id1, NULL, 0);
    for (i = 0; i < MAXACT; i++)
        ACTconnfd[i] = -1;
    ACTList = (struct sockaddr_in *) shmat (ACTsegment_id2, NULL, 0);
    ACTname = (struct user_name *) shmat (ACTsegment_id3, NULL, 0);

    AUDconnfd = (int *) shmat (AUDsegment_id1, NULL, 0);
    for (i = 0; i < MAXAUD; i++)
        AUDconnfd[i] = -1;
    AUDList = (struct sockaddr_in *) shmat (AUDsegment_id2, NULL, 0);
    AUDname = (struct user_name *) shmat (AUDsegment_id3, NULL, 0);
    
	
	MSGbuffer = (char *) shmat (MSGsegment, NULL, 0);
    PIPEflag = (int *) shmat (PIPEflagSegment, NULL, 0);

}

void MemAttach() {
    ACTconnfd = (int *) shmat (ACTsegment_id1, NULL, 0);
    ACTList = (struct sockaddr_in *) shmat (ACTsegment_id2, NULL, 0);
    ACTname = (struct user_name *) shmat (ACTsegment_id3, NULL, 0);

    AUDconnfd = (int *) shmat (AUDsegment_id1, NULL, 0);
    AUDList = (struct sockaddr_in *) shmat (AUDsegment_id2, NULL, 0);
    AUDname = (struct user_name *) shmat (AUDsegment_id3, NULL, 0);
}

void ACTServ() {

    int index;
    int psd;
    int rc;

    for(;;) {
        index = ACTgetfd();

        fromlen = sizeof(from);
        if (  (ACTconnfd[index]   = accept(ACTsd, (SA *)&from, &fromlen)) == -1) {
            if (errno == EINTR) continue;
        }

        if( (rc=recv(ACTconnfd[index] , username, sizeof(username), 0)) < 0)
            perror("receiving username  message");

        fromlen = sizeof (ACTList[index]);
        getpeername(ACTconnfd[index] ,(SA *)&ACTList[index],&fromlen);
        sprintf(ACTname[index].logname,"%s", username);

        printf("\nNew ACTOR  Client: (%s:%d) [%s]\n",
               inet_ntoa(ACTList[index].sin_addr), ntohs(ACTList[index].sin_port), ACTname[index].logname);

        ACTHandpid = fork ();
        if (ACTHandpid == 0) {
//            MemAttach();
            signal (SIGINT, SIG_IGN);
            signal (SIGQUIT, SIG_IGN);
            ACTHand (index);
        }
        printf ("Forked ACTOR Handler, PID =  %d \n", ACTHandpid);

    }
}

void AUDServ() {
    int index;
    int psd;
    int rc;

    for(;;) {
        fromlen = sizeof(from);
        if (  (psd  = accept(AUDsd, (SA *)&from, &fromlen)) == -1) {
            if (errno == EINTR) continue;
        }
        if( (rc=recv(psd, username, sizeof(username), 0)) < 0)
            perror("receiving username  message");
        printf("\nNew AUDIENCE  Client: (%s:%d) [%s]\n",
               inet_ntoa(from.sin_addr), ntohs(from.sin_port), username);
        index = AUDgetfd();
        AUDconnfd[index] = psd;
        fromlen = sizeof (AUDList[index]);
        getpeername(psd,(SA *)&AUDList[index],&fromlen);
        sprintf(AUDname[index].logname,"%s", username);
    }
}

int ACTgetfd ()
{
    int i;

    for (i = 0; i < MAXACT; i++) {
        if (ACTconnfd[i] == -1) return (i);
    }
    if (i == MAXACT) {
        printf ("MaxACT out fds\n");
        exit (-1);
    }
}

int AUDgetfd ()
{
    int i;

    for (i = 0; i < MAXAUD; i++) {
        if (AUDconnfd[i] == -1) return (i);
    }
    if (i == MAXAUD) {
        printf ("MaxAUD out fds\n");
        exit (-1);
    }
}

void ACTHand(int index) {

    int rc;
    char buf[512];
    int i;


    for (;;) {
        if ((rc = recv (ACTconnfd[index], buf, sizeof (buf), 0)) < 0)
        {
            ACTconnfd[i] = -1;;
            //perror ("receiving stream  message");
            exit (-1);
        }
        if (rc > 0)
        {
            buf[rc] = '\0';
            sprintf(MSGbuffer,"%s", buf);
            kill (getppid(), SIGUSR1);
            kill (AUDpid, SIGUSR2);
        }

    }
}

void RefreshLists() {
//printf("Refreshing\n");
    sprintf(MSGbuffer,"%s", " \b");
    kill (getpid(), SIGUSR1);
    kill (AUDpid, SIGUSR2);
}

void USR1handle(int sig)
{
    signal (SIGUSR1, USR1handle);
//printf("USR1 handling\n");

    *PIPEflag = 0;
    for (i = 0; i < MAXACT; i++) {
        if (ACTconnfd[i] != -1) {
            if (send (ACTconnfd[i], MSGbuffer, strlen(MSGbuffer), 0) < 0) {
                if (*PIPEflag == 1) printf("Actor: %d disconnected\n", i);
                ACTconnfd[i] = -1;
                // perror ("sending");
                // printf ("  ACT Client %d  Disconnected..", i);
            }
        }
    }

}
void USR2handle(int sig)
{
    signal (SIGUSR2, USR2handle);
//printf("USR2 handling\n");

    *PIPEflag = 0;
    for (i = 0; i < MAXAUD; i++) {
        if (AUDconnfd[i] != -1) {
            if (send (AUDconnfd[i], MSGbuffer, strlen(MSGbuffer), 0) < 0) {
                if (*PIPEflag == 1) printf("Audience: %d disconnected\n", i);
                AUDconnfd[i] = -1;
                //perror ("sending");
                //printf ("  AUD Client %d  Disconnected..", i);
            }
        }
    }
}

void reusePort (int s)
{
    int one = 1;

    if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof (one)) ==
            -1)
    {
        printf ("error in setsockopt,SO_REUSEPORT \n");
        exit (-1);
    }
}

void
PIPEhandle (int sig)
{
    signal (SIGPIPE, PIPEhandle);
    *PIPEflag = 1;
}
void
CHLDhandle (int sig)
{
    int cpid;
    signal (SIGCHLD, CHLDhandle);
    cpid=wait(NULL);
    //printf("ACTOR PID =  %d Terminated\n", cpid);
}

