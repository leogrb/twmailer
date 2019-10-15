/* myserver.c */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>

#include "include/ServerInputHelper.h"

extern pthread_mutex_t file_lock;
    pthread_t th1;
int main(int argc, char **argv)
{
    int create_socket, new_socket;
    socklen_t addrlen;
    char buffer[BUF];
    struct sockaddr_in address, cliaddress;
    pthread_t th1;
    thread_struct *thread_params = malloc(sizeof(thread_struct));
    if (argc < 3)
    {
        printf("Usage: %s port spooldirectory\n", argv[0]);
        return EXIT_FAILURE;
    }
    int port = atoi(argv[1]);
    char *spool = (char *)malloc(sizeof(char) * strlen(argv[2]));
    spool = argv[2];
    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating socket\n");
        free(spool);
        return EXIT_FAILURE;
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(create_socket, (struct sockaddr *)&address, sizeof(address)) != 0)
    {
        perror("bind error");
        free(spool);
        return EXIT_FAILURE;
    }
    if (listen(create_socket, 5) == -1)
    {
        perror("Listen to socket failed\n");
        free(spool);
        return EXIT_FAILURE;
    }

    addrlen = sizeof(struct sockaddr_in);

    //Create spool directory if non exisiting
    if (mkdir(spool, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
    {
        printf("Spool directory: %s successfully created\n", spool);
    }
    else if (errno == EEXIST)
    {
        // mkdir failed
    }
    else
    {
        perror("some other error");
    }
    while (true)
    {
        printf("Waiting for connections...\n");
        new_socket = accept(create_socket, (struct sockaddr *)&cliaddress, &addrlen);

        // handle new client
        if (new_socket > 0)
        {
            printf("Client connected from %s:%d...\n", inet_ntoa(cliaddress.sin_addr), ntohs(cliaddress.sin_port));
            strcpy(buffer, "Welcome to twmailer, Please enter your command\n");
            send(new_socket, buffer, strlen(buffer), 0);
            thread_params->spoolpath = spool;
            thread_params->socket_fd = &new_socket;
            thread_params->client_address = cliaddress;

            if(pthread_mutex_init(&file_lock, NULL) != 0)
            {
                perror("Mutex initialization error\n");
            }
            if(pthread_create(&th1, NULL, handle, (void *) thread_params) != 0)
            {
                perror("Error creating thread\n");
            }
        }
    }
    if(pthread_mutex_destroy(&file_lock) != 0){
        perror("Error destroying mutex\n");
    }
    close(create_socket);
    return EXIT_SUCCESS;
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char c, *ptr;
    ptr = vptr;
    for (n = 1; n < maxlen; n++)
    {
    again:
        if ((rc = read(fd, &c, 1)) == 1)
        {
            *ptr++ = c;
            if (c == '\n')
                break; // newline ist stored, like fgets()
        }
        else if (rc == 0)
        {
            if (n == 1)
                return (0); // EOF, no data read
            else
                break; // EOF, some data was read
        }
        else
        {
            if (errno == EINTR)
                goto again;
            return (-1); // error, errno set by read()
        };
    };
    *ptr = 0; // null terminate like fgets()
    return (n);
}//TO DO: format/comment code
