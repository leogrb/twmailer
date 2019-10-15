#include <netinet/in.h>

typedef struct thread_params
{
    char *spoolpath;
    int *socket_fd;
    struct sockaddr_in client_address;
} thread_struct;
