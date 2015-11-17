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

//init panel list shared mem
int segmentId,segment1, segment2, segment3, segment4;
client_machine *panel_member,*audience_member, *list_participants;
int *sharedInt_index;
char* client_msg;
int *QAsessionFlag;

Connection panel;
Connection audience;

char *byby="Session is Over\n";
pid_t actor_pid;
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
	
	for(;;){
        audience_pid = fork();
        
        /* This is the audience server */
        if ( audience_pid == 0) {
			
			signal (SIGQUIT, SIG_IGN);
			signal (SIGINT, SIG_IGN);
			signal (SIGTSTP,SIG_IGN);
			signal (SIGTERM,SIG_IGN);
			
			 	
			
            cout << "Forked AUDIENCE Server, PID=" << getpid() << endl;
            ServAudience(audience, client);
        }
        else
        {		
			//This is to handle the quit function
			if (signal(SIGQUIT, CloseMeeting) == SIG_ERR) {
				fprintf(stderr, "cannot set handler for SIGQUIT\n");
            }
			
			//list people in the room
			if (signal(SIGINT, inthandle) == SIG_ERR) {
                fprintf(stderr, "cannot set handler for SIGINT\n");
            }
			
			//Handle dead signal
            if (signal(SIGTERM, SomeoneIsLeaving) == SIG_ERR) {
                fprintf(stderr, "cannot set handler for SIGTERM\n");
            }
			
			//Handle ctrl+Z 
			 if (signal(SIGTSTP, AllowQA) == SIG_ERR) {
                fprintf(stderr, "cannot set handler for SIGTSTP\n");
            }
			
            cout << "AUDIENCE server started at port: (" << ntohs(audience.server.sin_port) << ")" << endl;
            cout << "PANELIST server started at port: (" << ntohs(panel.server.sin_port) << ")" << endl;          
			ServPanelist(panel, client);
        }
       wait(NULL); 
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
            perror("receiving stream  message");
            exit(-1);
        }
        if (rc > 0){
			buf[rc]='\0';
			sprintf(client_msg,"%s", buf);
            if ( purpose == GETUSERNAME )
			{
				break;
				//First message to client
				//sprintf(client_msg,"%s", "Welcome to the session!\n");
				//if ((send(buffer_stream, client_msg, MESSAGE_SIZE, 0) <0 ) && (i<=SIZE))
				//	perror("sending stream message");
				
				//Send a message to this client if QA session opened 		
				if (*QAsessionFlag == 1)	//QA session open
					{
						sprintf(client_msg,"%s", "We are in the QA session!\n");
						if ((send(buffer_stream, client_msg, MESSAGE_SIZE, 0) <0 ) && (i<=SIZE))
					perror("sending stream message");
					}
				sprintf(client_msg,"%s", buf);
				
			}
			if (purpose == SERVPANEL)
			{
				raise(SIGUSR1);
				break;
			}
			
			if (purpose == SERVAUDIENCE)
				{
					raise(SIGUSR1);
					//cout << "current audience pid " << getpid();
					int tmpIndex = getIndexByPID (getpid());					
					if (*QAsessionFlag == 0) //QA is not allow yet				
					{
						//sprintf(client_msg,"%s", "QA is not allowed yet");
						//if ((send(buffer_stream, client_msg, MESSAGE_SIZE, 0) <0 ) && (i<=SIZE))
							//perror("sending stream message");
						sprintf(client_msg,"%s", buf);
						//break;
					}
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

int getIndexByPID (int v_pid)
{
	int index = *sharedInt_index;
	for (int i=1;i<=index;i++)
	{
		if (list_participants[index].pid == v_pid)
			return i;
	}
	return -1; //error
}

void ServPanelist ( Connection panelist, struct sockaddr_in client)
{
    int psd, index=0;
    pid_t childpid;
    socklen_t t = sizeof(panelist.server);
 
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
    }
}

void ServAudience ( Connection audience, struct sockaddr_in client)
{
    int psd, childpid, index=0;
    socklen_t t = sizeof(audience.server);
	
	    
    for(;;){
        psd  = accept(audience.socket_s, (struct sockaddr *) &client, &t);
        EchoServe(psd, client, GETUSERNAME);
        childpid = fork();
        if ( childpid == 0) {
			signal (SIGQUIT, SIG_IGN);
			signal (SIGINT, SIG_IGN);		
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
			
			signal (SIGQUIT, SIG_IGN);
			signal (SIGINT, SIG_IGN);		
			signal (SIGTSTP,SIG_IGN); 
			
	 		cout << "\nNew AUDIENCE Client: (" << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port) <<
                    ") [" << client_msg << "]" << endl;
            cout << "Forked AUDIENCE Handler, PID = " << childpid << endl;
			
			*sharedInt_index = *sharedInt_index + 1 ;
			index = *sharedInt_index;
			//cout << "new parti " << index <<endl;
			list_participants[index].client_type = PARTICIPANT_AUD;
			list_participants[index].pid = childpid;
			list_participants[index].port = client.sin_port;
			list_participants[index].address = client.sin_addr;
			list_participants[index].buf_stream = psd;
			strcpy(list_participants[index].user_name, client_msg);
			list_participants[index].time_in = time (NULL);
        }
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

void inthandle (int sig)
{
	int i, index;
    signal (SIGINT, inthandle);
    struct sockaddr_in myActor;
	index = *sharedInt_index; 
	
	//Panel list
	cout << "\n\n Current Panelist" << endl;
    cout << "PID     Address              Username       Type        Time In      Time Out " << endl;
    cout << "-----  ----------------     ----------     ---------    ----------    ---------" << endl;
	print_participants(index, PARTICIPANT_PAN);
	
	//Audience list
	cout << "\n\n Current Audience" << endl;
    cout << "PID     Address              Username       Type        Time In      Time Out " << endl;
    cout << "-----  ----------------     ----------     ---------    ----------    ---------" << endl;
	print_participants(index, PARTICIPANT_AUD);
	
	//Audience and panelist have left
	cout << "\n\n Audience and Panelist who have left" << endl;
    cout << "PID     Address              Username       Type                 Time In         Time Out " << endl;
    cout << "-----  ----------------     ----------     ---------           ----------        ---------" << endl;
	print_participants(index, PARTICIPANT_PAN_LEFT);
	print_participants(index, PARTICIPANT_AUD_LEFT);
	
	cout << "\n\n";
}

void print_participants(int index, int p_type)
//#define PARTICIPANT_AUD    	   1
//#define PARTICIPANT_PAN        2
//#define PARTICIPANT_AUD_LEFT   3
//#define PARTICIPANT_PAN_LEFT   4
{
	int i =1;
	 while ( i<= index )
    {
		if (list_participants[i].client_type == p_type)
		{
			cout << list_participants[i].pid << "\t" 
			<< inet_ntoa(list_participants[i].address) << ":"
			<< ntohs(list_participants[i].port ) << "\t\t" 
			<<(list_participants[i].user_name) << "\t";
			
			switch ( list_participants[i].client_type ) {
			case PARTICIPANT_AUD:            // Note the colon, not a semicolon
				cout << "   Audience\t";
				break;
			case PARTICIPANT_PAN:            // Note the colon, not a semicolon
				cout << "   Panelist\t";
				break;
			case PARTICIPANT_PAN_LEFT:            // Note the colon, not a semicolon
				cout << "   Panelist (Left)\t";
				break;
			case PARTICIPANT_AUD_LEFT:            // Note the colon, not a semicolon
				cout << "   Audience (Left)\t";
				break;				
			default:            // Note the colon, not a semicolon
				break;
			}
			cout << print_time(list_participants[i].time_in) << "\t";		
			if (list_participants[i].time_out)
			cout<< print_time(list_participants[i].time_out) << "\t"; 		
			cout << endl;
		}
        i++;
    }	
}

void displayHandle (int sig)
{
    signal (SIGUSR1, displayHandle);
    int signal_sent = 0;
    int i=1;
    // send message to all participants
    while ( i<=SIZE && list_participants[i].pid != 0 )
    {
        signal_sent = kill(list_participants[i].pid , SIGUSR2);
        i++;
    }
}

void SendMessage (int sig)
{
    signal (SIGUSR2, SendMessage);

    int i=1;
    int myPSD = 0;
    pid_t myPID = getpid();
    // look current process in array to send message
    while ( (i<=SIZE) &&  (list_participants[i].pid != 0) )
    {
        if (( list_participants[i].pid == myPID )  )
        {
            myPSD = list_participants[i].buf_stream;
			if ((send(myPSD, client_msg, MESSAGE_SIZE, 0) <0 ) && (i<=SIZE))
				perror("sending stream message");
            break;
        }
        i++;
    }
}

void AllowQA(int sig)
{
	int signal_sent;
	signal(SIGTSTP, AllowQA);

		if (*QAsessionFlag ==0) 
		{
		
		sprintf(client_msg,"%s", "Now is the time for QA session!");
		int index = *sharedInt_index;
		//Sending closing message to everyone
		for (int i=1;i<=index;i++)
			{
				signal_sent = kill(list_participants[i].pid , SIGUSR2);
			}
		}
		else
		{
			sprintf(client_msg,"%s", "Now is the time for QA session!");
			
		}
		
		*QAsessionFlag = 1;
 
}
 
void SomeoneIsLeaving( int sig )
{
    signal(SIGTERM, SomeoneIsLeaving);
	int i,v_pid;
	v_pid = getpid();
	int index = *sharedInt_index; 
	
	for (i=1;i<=index;i++)
	{
		if (list_participants[i].pid == v_pid)
		{
			//Set status that this participant has left
			if (list_participants[i].client_type == PARTICIPANT_AUD )
				list_participants[i].client_type = PARTICIPANT_AUD_LEFT;
			
			//Set status that this participant has left
			if (list_participants[i].client_type == PARTICIPANT_PAN )
				list_participants[i].client_type = PARTICIPANT_PAN_LEFT;
			
			//Set the time that they left the session
			list_participants[i].time_out =time (NULL);
			
			//Give a notice to server
			printf("Participant [%d] just left the room! \n",getpid());
			break;
		}
	}
}

void CloseMeeting (int sig)
{
	int signal_sent;
    signal (SIGQUIT, CloseMeeting);
    printf("\n\n Closing session. Please wait ... \n");
    
	//Set closing message to share memory
	sprintf(client_msg,"%s", byby);
    
	int index = *sharedInt_index;
	//Sending closing message to everyone
	for (int i=1;i<=index;i++)
	{
		signal_sent = kill(list_participants[i].pid , SIGUSR2);
	}
	kill (audience_pid, SIGUSR2);
	sleep(2);
    system("pkill -9 panserver");
    exit(0);
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

void ignoreme (int sig)
{
    signal(SIGINT, ignoreme);
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