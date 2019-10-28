#include <netinet/in.h>
#include "vector.h"

typedef struct thread_params
{
    char *spoolpath;
    int *socket_fd;
    struct sockaddr_in client_address;
    vector *vec;
} thread_struct;
