#ifndef SHAREMEM_H_INCLUDED
#define SHAREMEM_H_INCLUDED

#define USER_SIZE      25

struct TheaterClients
{
    pid_t pid;
    char user_name[USER_SIZE];
    struct in_addr address;
    int buf_stream;
};


#endif // SHAREMEM_H_INCLUDED
