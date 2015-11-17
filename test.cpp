// accessing mapped values
#include "header.h"
#include <iostream>
#include <map>
#include <string>
#define MAXHOSTNAME 80

// Required by for routine
 
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>   // Declaration for exit()

int pan_socket;
char ThisHost[80];


struct client_host
{
	std::string ip_address;
	std::string host_name;
};

int main ()
{
int pan_pid, mypid;
  std::map<int,client_host> map_ip;
  std::map<int,client_host>::iterator it;

  pan_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  listen (pan_socket, 4);
  gethostname(ThisHost, MAXHOSTNAME);
  
  cout<<ThisHost<<endl;
  
  pan_pid = fork ();
  
if (pan_pid ==0)
	{
		mypid = getpid();
	    map_ip[pan_pid].ip_address = "localhost";
		std::cout << "map_ip now contains " << map_ip.size() << " elements.\n";
	}
else
	{
		//map_ip[pan_pid].ip_address = "localhost";
  //if !find_element()
  //map_ip[pan_pid]="localhost";
  
  
  // it = map_ip.find('d');
  //if (it != map_ip.end())
 // {
//	
 //   map_ip.erase (it);
 // }

  
	 
	}
  return 0;
}