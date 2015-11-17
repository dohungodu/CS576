#include "header.h"
#include "Connection.cpp" // Connection Object
#include <ctime>
#define PARTICIPANT_FREE       0
#define PARTICIPANT_AUD    	   1
#define PARTICIPANT_PAN        2
#define PARTICIPANT_AUD_LEFT   3
#define PARTICIPANT_PAN_LEFT   4
#define SA struct sockaddr
struct client_machine
{
    pid_t pid;
    char user_name[USER_SIZE];
    struct in_addr address;
    u_short port;
    int buf_stream;
	int client_type;
	time_t time_in;
	time_t time_out;
};

void EchoServe( int buffer_stream, struct sockaddr_in client, int purpose=SERVPANEL);
void ServPanelist ( Connection actors, struct sockaddr_in client);
void ServAudience ( Connection audience, struct sockaddr_in client);
void GetUserName( int buffer_stream, struct sockaddr_in client);
void exit(int);
void displayHandle (int sig);
void reusePort(int sock);
void inthandle (int sig);
void ignoreme (int sig);
void SendMessage ( int sig);
void SomeoneIsLeaving( int sig );
void CloseMeeting(int sig);
void AllowQA(int sig);
void initSharedMem();
void print_participants(int index, int p_type);
int getIndexByPID (int v_pid);
void PIPEhandler(int sig);
void checkParticipant(int index,int par_type);

//init panel list shared mem
int segmentId,segment1, segment2, segment3, segment4;
client_machine *panel_member,*audience_member, *list_participants;
int *sharedInt_index;
char* client_msg;
int *QAsessionFlag;

Connection panel;
Connection audience;

char *byby="Session is Over\n";
pid_t panel_pid;
pid_t audience_pid;

int main(int argc, char* argv[])
{
    struct sockaddr_in client;

	//Validate input
    if (argc != 2)
    {
        cout << "Usage: panserver <portnumber>" << endl;
        return -1;
    }	
	// initialize shared memory
	initSharedMem();
    // Initialize the ports
    panel.server.sin_port = htons(atoi(argv[1]));
    audience.server.sin_port = htons(atoi(argv[1])+1);

    /** Create socket on which to send  and receive */

    if ( panel.CreatSocket() < 0 ) // create panel socket
        return -1;

    if ( audience.CreatSocket() < 0 ) // create panel socket
        return -1;
	
    reusePort(panel.socket_s); //reuse the binding port when execute program
    
	if ( panel.BindConnection() < 0 ) // Check for binding errors
        return -1;
    
	reusePort(audience.socket_s);//reuse the binding port when execute program
	
    if ( audience.BindConnection() < 0 ) // Check for binding errors
        return -1;

    /** get panel port information and prints it out */
    if ( panel.GetSocketName() ) // check socket name validity
        return -1;

    /** get audience port information and prints it out */
    if ( audience.GetSocketName() ) // check socket name validity
        return -1;
	
    /** accept TCP connections from clients and fork a process to serve each */
    listen(panel.socket_s,10);
    listen(audience.socket_s,10);
	
	signal(SIGPIPE, PIPEhandler);
	
	
	for(;;){
        audience_pid = fork();
		signal (SIGINT, SIG_IGN);
		signal (SIGTSTP,SIG_IGN); 
        
        /* This is the audience server */
        if ( audience_pid == 0) {
		        cout << "Forked AUDIENCE Server, PID=" << getpid() << endl;
				
				ServPanelist(panel, client);
        }
        else
        {
			panel_pid = fork ();
			signal (SIGINT, SIG_IGN);
			signal (SIGTSTP,SIG_IGN); 
			//signal(SIGPIPE, PIPEhandler);
			
			if ( panel_pid == 0) {
			    cout << "Forked PANEL Server, PID=" << getpid() << endl;
				ServAudience(audience, client);
			}	
			else
			{
				//list people in the room
				
				cout << "AUDIENCE server started at port: (" << ntohs(audience.server.sin_port) << ")" << endl;
				cout << "PANELIST server started at port: (" << ntohs(panel.server.sin_port) << ")" << endl;  
				
					
			}
			wait(NULL); 
        }
	   sleep(2);
       wait(NULL); 
    }
}


void ServPanelist ( Connection panelist, struct sockaddr_in client)
{
    int psd, index=0;
    pid_t childpid;
	int *status;
	
    socklen_t t = sizeof(panelist.server);
	
	signal(SIGPIPE, PIPEhandler);
	
    for(;;){
        psd  = accept(panelist.socket_s, (struct sockaddr *) &client, &t);
        EchoServe(psd, client, GETUSERNAME);
        childpid = fork();
        if ( childpid == 0) {
			signal (SIGQUIT, SIG_IGN);
			signal (SIGINT, SIG_IGN);		
			signal (SIGTSTP,SIG_IGN); 

            if (signal(SIGUSR1, displayHandle) == SIG_ERR) {
                fprintf(stderr, "cannot set handler for SIGUSR1\n");
            }

            if (signal(SIGUSR2, SendMessage) == SIG_ERR) {
                fprintf(stderr, "cannot set handler for SIGUSR2\n");
            }

            close (panelist.socket_s);
            EchoServe(psd, client, SERVPANEL);
        }
        else{
            
			signal (SIGQUIT, SIG_IGN);
			signal (SIGINT, SIG_IGN);		
			signal (SIGTSTP,SIG_IGN); 

            cout << "\nNew Panel Client: (" << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port) <<
                    ") [" << client_msg << "]" << endl;
            cout << "Forked Panel Handler, PID = " << childpid << endl;
			
			*sharedInt_index = *sharedInt_index + 1 ;
			index = *sharedInt_index;
			
			//cout << "new parti " << index <<endl;
			list_participants[index].client_type = PARTICIPANT_PAN;
			list_participants[index].pid = childpid;
			list_participants[index].port = client.sin_port;
			list_participants[index].address = client.sin_addr;
			list_participants[index].buf_stream = psd;
			strcpy(list_participants[index].user_name, client_msg);		
			// current date/time based on current system
			list_participants[index].time_in = time (NULL);		
        }

		//Handle the child signal just end. Meaning of someone left the room. Pipe broken.
		int pid=wait(&status);	
		//#define PARTICIPANT_AUD    	   1
		//#define PARTICIPANT_PAN        2
		//checkParticipant(index,PARTICIPANT_PAN);	
    }
}

void ServAudience ( Connection audience, struct sockaddr_in client)
{
    int psd, childpid, index=0;
    socklen_t t = sizeof(audience.server);
	int *status;
	
    for(;;){
		psd  = accept(audience.socket_s, (struct sockaddr *) &client, &t);
        EchoServe(psd, client, GETUSERNAME);
        childpid = fork();
        if ( childpid == 0) {
			signal (SIGTSTP,SIG_IGN);
			if (signal(SIGUSR1, displayHandle) == SIG_ERR) {
                fprintf(stderr, "cannot set handler for SIGUSR1\n");
            }
            
			// getting interrupt to send an audience message
            if (signal(SIGUSR2, SendMessage) == SIG_ERR) {
                fprintf(stderr, "cannot set handler for SIGUSR2\n");
            }
            
			close (audience.socket_s);
            EchoServe(psd, client, SERVAUDIENCE);
			
        }
        else{
			signal (SIGTSTP,SIG_IGN);
			cout << "\nNew audience Client: (" << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port) <<
                    ") [" << client_msg << "]" << endl;
            cout << "Forked audience Handler, PID = " << childpid << endl;
			*sharedInt_index = *sharedInt_index + 1 ;
				index = *sharedInt_index;
				
				//cout << "new parti " << index <<endl;
				list_participants[index].client_type = PARTICIPANT_AUD;
				list_participants[index].pid = childpid;
				list_participants[index].port = client.sin_port;
				list_participants[index].address = client.sin_addr;
				list_participants[index].buf_stream = psd;
				strcpy(list_participants[index].user_name, client_msg);		
				// current date/time based on current system
				list_participants[index].time_in = time (NULL);	
				
				//if Q&A session opened send a message
				if (*QAsessionFlag == 1)
				{
					cout << "We are in Q&A session now!" << endl;
					sprintf(client_msg,"%s", "We are in Q&A session now!");
						if ( send(list_participants[index].buf_stream, client_msg, strlen(client_msg), 0) < 0)
							 
								perror("Error when sending message Q&A to client ");
						 
				}
        }
	
		//Handle the child signal just end. Meaning of someone left the room. Pipe broken.
		int pid=wait(&status);	
		//#define PARTICIPANT_AUD    	   1
		//#define PARTICIPANT_PAN        2
		//checkParticipant(index,PARTICIPANT_AUD);
	 
    }
}




std::string print_time (time_t v_time)

{
	struct tm *p;
	p = gmtime ( &v_time);
	char s[25];
	sprintf(s, "%02d:%02d:%02d", p->tm_hour, p->tm_min, p->tm_sec);
	return s;
}


void SendMessage (int sig)
{
    signal (SIGUSR2, SendMessage);

    int i;
	int index = *sharedInt_index; 
    int myPSD = 0;
    pid_t myPID = getpid();
    // look current process in array to send message
    for (i=1;i<=index;i++)
    {
		if (list_participants[i].client_type<=2) //current participant
        {
			cout << list_participants[i].client_type << i << endl;
            myPSD = list_participants[i].buf_stream;
			if ((send(myPSD, client_msg, strlen(client_msg), 0) <0 ) && (i<=SIZE))
				perror("sending stream message");
            break;
        }
    }
}



void initSharedMem()
{
	int i;
	
	segment1= shmget(IPC_PRIVATE, SIZE * sizeof(client_machine), S_IRUSR | S_IWUSR);
	list_participants = (client_machine *) shmat(segment1, NULL, 0);

	//a shared memory for message
	segment4 = shmget(IPC_PRIVATE, MESSAGE_SIZE, S_IRUSR | S_IWUSR);
	client_msg = (char *) shmat(segment4,NULL,0);
	
	//To count all participants
	segmentId = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR | S_IWUSR); 
	sharedInt_index = (int *) shmat(segmentId, NULL, 0);
	
	//To check QA session is open or not
	segment2 = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR | S_IWUSR); 
	QAsessionFlag = (int *) shmat(segment2, NULL, 0);
	
	*sharedInt_index = 0;
	*QAsessionFlag = 0;
	for (int i = 0; i < SIZE; i++)
		{
			list_participants[i].client_type = PARTICIPANT_FREE;
		}
}

void EchoServe( int buffer_stream, struct sockaddr_in client, int purpose)
{
    char buf[512];
    int rc, i;
    struct  hostent *host_info, *gethostbyname();
    struct servent *serv_info;

    if ((host_info = gethostbyaddr((char *)&client.sin_addr.s_addr,
        sizeof(client.sin_addr.s_addr),AF_INET)) == NULL)
        fprintf(stderr, "Can't find host %s\n", inet_ntoa(client.sin_addr));

    /**  get data from  clients and send it back */
    for(;;){
        if( (rc=recv(buffer_stream, buf, sizeof(buf), 0)) < 0){
            perror("11receiving stream  message");
            exit(-1);
        }
        if (rc > 0){
			buf[rc]='\0';
			sprintf(client_msg,"%s", buf);
            if ( purpose == GETUSERNAME )
			{
				
				break;
							
			}
			if (purpose == SERVPANEL)
			{
				raise(SIGUSR1);
				//break;
			}
			
			if (purpose == SERVAUDIENCE)
				{
					raise(SIGUSR1);
				}										
        }
        else {
            raise(SIGTERM);
            close (buffer_stream);
            exit(0);
        }
    }
    return;
}

 



void reusePort(int s)
{
    int one=1;

    if ( setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char *) &one,sizeof(one)) == -1 )
    {
        printf("error in setsockopt,SO_REUSEPORT \n");
        exit(-1);
    }
}