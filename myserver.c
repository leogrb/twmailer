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

#include "include/ServerInputHelper.h"

ssize_t readline(int fd, void *vptr, size_t maxlen);

int main(int argc, char **argv)
{
    int create_socket, new_socket;
    socklen_t addrlen;
    char buffer[BUF];
    char user[10];
    char receiver[10];
    char subject[82];
    char DELnumbr[2];
    int size, valid;
    struct sockaddr_in address, cliaddress;
    if (argc < 3)
    {
        printf("Usage: %s port spooldirectory\n", argv[0]);
        return EXIT_FAILURE;
    }
    int port = atoi(argv[1]);
    char *spool = (char *)malloc(sizeof(char) * strlen(argv[2]));
    spool = argv[2];
    if((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
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
    if(listen(create_socket, 5) == -1)
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
        if (new_socket > 0)
        {
            printf("Client connected from %s:%d...\n", inet_ntoa(cliaddress.sin_addr), ntohs(cliaddress.sin_port));
            strcpy(buffer, "Welcome to twmailer, Please enter your command\n");
            send(new_socket, buffer, strlen(buffer), 0);
        }
        do
        {
            size = readline(new_socket, buffer, BUF - 1);
            if (size > 0)
            {
                buffer[size] = '\0';
                printf("Message received: %s", buffer);
                if ((strncmp(buffer, "SEND", 4)) == 0)
                {
                    printf("Processing SEND request\n");
                    // process message
                    for (int i = 0; i < 4; i++)
                    {
                        valid = -1;
                        size = readline(new_socket, buffer, BUF - 1);
                        if (size > 0 && size < 10 && i < 2)
                        {
                            buffer[size] = '\0';
                            if (i == 0)
                            {
                                strcpy(user, buffer);
                                printf("Request from user: %s", user);
                                valid = 1;
                            }
                            else if (i == 1)
                            {
                                strcpy(receiver, buffer);
                                printf("Send to receiver: %s", receiver);
                                valid = 1;
                            }
                        }
                        else if (size > 0 && size < 82 && i == 2)
                        {
                            buffer[size] = '\0';
                            strcpy(subject, buffer);
                            printf("Mail subject: %s", subject);
                            valid = 1;
                        }
                        else if (size > 0 && i == 3)
                        {
                            //linked list for saving msg text
                            buffer[size] = '\0';
                            msg *head = malloc(sizeof(msg));
                            head->next = NULL;
                            strcpy(head->text, buffer);
                            do
                            {
                                
                                size = readline(new_socket, buffer, BUF - 1);
                                if (size > 0)
                                {
                                    buffer[size] = '\0';
                                    if (size != 2 && buffer[0] != '.' && buffer[1] != '\n')
                                    {
                                        push(head, buffer);
                                    }
                                    valid = 1;
                                }
                                else
                                {
                                    freeList(head);
                                    valid = -1;
                                    break;
                                }
                            } while (size != 2 && buffer[0] != '.' && buffer[1] != '\n');
                            if (valid == 1)
                            {
                                //save msg here
                                if (createmsg(user, receiver, subject, head, spool))
                                {
                                    printf("Request successfully executed\n");
                                    strcpy(buffer, "OK\n");
                                }
                                else
                                {
                                    printf("Request execution failed\n");
                                    strcpy(buffer, "ERR\n");
                                }
                                freeList(head);
                                send(new_socket, buffer, strlen(buffer), 0);
                            }
                            buffer[size] = '\0';
                        }
                        else if (valid != 1)
                        {
                            printf("Request execution failed\n");
                            strcpy(buffer, "ERR\n");
                            send(new_socket, buffer, strlen(buffer), 0);
                            break;
                        }
                    }
                }
                else if ((strncmp(buffer, "LIST", 4)) == 0)
                {
                    printf("Processing LIST request\n");
                    size = readline(new_socket, buffer, BUF - 1);
                    bool isValid = false;
                    if (size > 0 && size < 10)
                    {
                        strcpy(user, buffer);
                        printf("Request from user: %s", user);
                        char messages[BUF];
                        if (listAllMessages(spool, user, messages))
                        {
                            printf("Request successfully executed\n");
                            strcpy(buffer, messages);
                            strcat(buffer, "OK\n");
                            send(new_socket, buffer, strlen(buffer), 0);
                            isValid = true;
                        }
                    }
                    if (!isValid)
                    {
                        printf("Request execution failed\n");
                        strcpy(buffer, "ERR\n");
                        send(new_socket, buffer, strlen(buffer), 0);
                    }
                }
                else if ((strncmp(buffer, "READ", 4)) == 0)
                {
                    printf("Processing READ request\n");
                    size = readline(new_socket, buffer, BUF - 1);
                    bool isValid = false;
                    char output[BUF];
                    if (size > 0 && size < 10)
                    {
                        strcpy(user, buffer);
                        printf("Request from user: %s", user);
                        size = readline(new_socket, buffer, BUF - 1);
                        if (size > 0)
                        {
                            int n = 0;
                            int msgNumber;
                            while (n < size && isdigit(buffer[n]))
                            {
                                n++;
                            }
                            if (n == 0)
                            {
                                isValid = false;
                            }
                            if (n == 1)
                            {
                                char readNumber[2];
                                readNumber[0] = buffer[0];
                                readNumber[1] = '\n';
                                msgNumber = (int)strtol(readNumber, NULL, 10);
                                if (msgNumber != 0)
                                {
                                    isValid = readMessage(user, msgNumber, spool, output);
                                }
                            }
                            else if (n > 1 && buffer[n] == '\n')
                            {
                                buffer[n] = '\0';
                                msgNumber = (int)strtol(buffer, NULL, 10);
                                isValid = readMessage(user, msgNumber, spool, output);
                            }
                        }
                    }
                    if (!isValid)
                    {
                        printf("Request execution failed\n");
                        strcpy(buffer, "ERR\n");
                        send(new_socket, buffer, strlen(buffer), 0);
                    }
                    else
                    {
                        printf("Request successfully executed\n");
                        strcpy(buffer, output);
                        strcat(buffer, "OK\n");
                        send(new_socket, buffer, strlen(buffer), 0);
                    }
                }
                else if ((strncmp(buffer, "DEL", 3)) == 0)
                {
                    printf("Processing DELETE request\n");
                    size = readline(new_socket, buffer, BUF - 1);
                    bool DELvalid = false;
                    if (size > 0 && size < 10)
                    {
                        strcpy(user, buffer);
                        printf("Request from user: %s", user);
                        size = readline(new_socket, buffer, BUF - 1);
                        if (size > 0)
                        {
                            int n = 0;
                            int msgnr;
                            while (n < size && isdigit(buffer[n]))
                            {
                                n++;
                            }
                            if (n == 0)
                            {
                                DELvalid = false;
                            }
                            if (n == 1)
                            {
                                DELnumbr[0] = buffer[0];
                                DELnumbr[1] = '\0';
                                msgnr = (int)strtol(DELnumbr, NULL, 10);
                                if (msgnr != 0)
                                {
                                    DELvalid = deletemsg(user, msgnr, spool);
                                }
                            }
                            else if (n > 1 && buffer[n] == '\n')
                            {
                                buffer[n] = '\0';
                                msgnr = (int)strtol(buffer, NULL, 10);
                                DELvalid = deletemsg(user, msgnr, spool);
                            }
                        }
                    }
                    if (!DELvalid)
                    {
                        printf("Request execution failed\n");
                        strcpy(buffer, "ERR\n");
                        send(new_socket, buffer, strlen(buffer), 0);
                    }
                    else
                    {
                        printf("Request successfully executed\n");
                        strcpy(buffer, "OK\n");
                        send(new_socket, buffer, strlen(buffer), 0);
                    }
                }
            }
            else if (size == 0)
            {
                printf("Client closed remote socket\n");
                break;
            }
            else
            {
                perror("recv error");
                return EXIT_FAILURE;
            }
        } while (strncmp(buffer, "quit", 4) != 0);
        close(new_socket);
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
}