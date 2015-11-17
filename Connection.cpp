class Connection
{
public:
    Connection()
    {
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
    //  server.sin_port = htons(atoi(argv[1]));      ** set in main()
    }
    ~Connection(){}

    // Create a Socket for a connection
    int CreatSocket()
    {
        socket_s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        /* OR audience_socket = socket (host_info->h_addrtype,SOCK_STREAM,0); */
        if (socket_s<0) {
            perror("opening stream socket");
        }

        return socket_s;
    }

    // Binds Socket to connection
    int BindConnection()
    {
        int action = bind( socket_s, (struct sockaddr *) &server, sizeof(server) );
        if ( action < 0 ) {
            close(socket_s);
            perror("binding name to stream socket");
        }

        return action;
    }

    /** get actor port information and  prints it out */
    int GetSocketName()
    {
        socklen_t t = sizeof(server);
        int socket_name = getsockname (socket_s, (struct sockaddr *)&server, &t);

        if ( socket_name ) {
            perror("getting socket name");
        }

        return socket_name;
    }
 
    /** Server socket variables*/
    int socket_s;
    struct sockaddr_in server;
};
