#ifndef THEATERCLIENTS_H_INCLUDED
#define THEATERCLIENTS_H_INCLUDED

struct TheaterClients
{
    pid_t pid;
    char user_name[USER_SIZE];
    struct in_addr address;
    u_short port;
    int buf_stream;
};


#endif // THEATERCLIENTS_H_INCLUDED
