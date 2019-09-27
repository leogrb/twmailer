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

#include "inputHelper.h"

typedef struct node
{
    char text[BUF];
    struct node *next;
} msg;

void push(msg *head, char msgtext[]);
void freeList(msg *head);
bool createmsg(char *user, char *receiver, char *subject, msg *head, char *spool);
int counter(char* userdir);
ssize_t readline(int fd, void *vptr, size_t maxlen);

int main(int argc, char **argv)
{
    int create_socket, new_socket;
    socklen_t addrlen;
    char buffer[BUF];
    char user[10];
    char receiver[10];
    char subject[82];
    int size;
    int valid;
    struct sockaddr_in address, cliaddress;
    if (argc < 3)
    {
        printf("Usage: %s port spooldirectory\n", argv[0]);
        return EXIT_FAILURE;
    }
    int port = atoi(argv[1]);
    printf("%d", port);
    char *spool = (char *)malloc(sizeof(char) * strlen(argv[2]));
    spool = argv[2];
    create_socket = socket(AF_INET, SOCK_STREAM, 0);

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(create_socket, (struct sockaddr *)&address, sizeof(address)) != 0)
    {
        perror("bind error");
        return EXIT_FAILURE;
    }
    listen(create_socket, 5);

    addrlen = sizeof(struct sockaddr_in);

    //Create spool directory if non exisiting
    if (mkdir(spool, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
    {
        printf("%s ir successfully created", spool);
    }
    else if (errno == EEXIST)
    {
        // mkdir failed
        printf("dir already exists");
    }
    else
    {
        printf("some other error");
    }
    while (true)
    {
        printf("Waiting for connections...\n");
        new_socket = accept(create_socket, (struct sockaddr *)&cliaddress, &addrlen);
        if (new_socket > 0)
        {
            printf("Client connected from %s:%d...\n", inet_ntoa(cliaddress.sin_addr), ntohs(cliaddress.sin_port));
            strcpy(buffer, "Welcome to myserver, Please enter your command:\n");
            send(new_socket, buffer, strlen(buffer), 0);
        }
        do
        {
            size = readline(new_socket, buffer, BUF - 1);
            if (size > 0)
            {
                buffer[size] = '\0';
                printf("%d", size);
                printf("Message received: %s", buffer);
                if ((strncmp(buffer, "SEND", 4)) == 0)
                {
                    // process message
                    for (int i = 0; i < 4; i++)
                    {
                        printf("%d", i);
                        valid = -1;
                        size = readline(new_socket, buffer, BUF - 1);
                        if (size > 0 && size < 10 && i < 2)
                        {
                            buffer[size] = '\0';
                            if (i == 0)
                            {
                                strcpy(user, buffer);
                                printf("user: %s", user);
                                valid = 1;
                            }
                            else if (i == 1)
                            {
                                strcpy(receiver, buffer);
                                printf("receiver: %s", receiver);
                                valid = 1;
                            }
                        }
                        else if (size > 0 && size < 82 && i == 2)
                        {
                            buffer[size] = '\0';
                            strcpy(subject, buffer);
                            printf("user: %s subject: %s", user, subject);
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
                                // fix: if msg is empty (just '\n'), '\n.' is still stored
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
                                    strcpy(buffer, "OK\n");
                                }
                                else
                                {
                                    strcpy(buffer, "ERR\n");
                                }
                                freeList(head);
                                send(new_socket, buffer, strlen(buffer), 0);
                            }
                            buffer[size] = '\0';
                        }
                        else if (valid != 1)
                        {
                            printf("%d", i);
                            strcpy(buffer, "ERR\n");
                            send(new_socket, buffer, strlen(buffer), 0);
                            break;
                        }
                    }
                }
                else if ((strncmp(buffer, "LIST", 4)) == 0)
                {
                    size = readline(new_socket, buffer, BUF - 1);
                    bool isValid = false;
                    if (size > 0 && size < 10)
                    {
                        strcpy(user, buffer);
                        printf("user: %s", user);
                        char messages[BUF];
                        if (listAllMessages(spool, user, messages))
                        {
                            strcpy(buffer, messages);
                            strcat(buffer, "OK\n");
                            send(new_socket, buffer, strlen(buffer), 0);
                            isValid = true;
                        }
                    }
                    if (!isValid)
                    {
                        strcpy(buffer, "ERR\n");
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

void push(msg *head, char msgtext[])
{
    msg *current = head;
    if (head->text[0] != '\0')
    {
        while (current->next != NULL)
        {
            current = current->next;
        }

        /* now we can add a new variable */
        current->next = malloc(sizeof(msg));
        strcpy(current->next->text, msgtext);
        current->next->next = NULL;
    }
    else
    {
        strcpy(head->text, msgtext);
    }
}

void freeList(msg *head)
{
    msg *tmp;
    while (head != NULL)
    {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}

bool createmsg(char *user, char *receiver, char *subject, msg *head, char *spool)
{
    int fd;
    char temp[BUF];
    receiver[strlen(receiver) - 1] = '\0'; //get rid of '\n'
    //build folder for receiver
    strcpy(temp, spool);
    strcat(temp, "/");
    strcat(temp, receiver);
    //Create user directory if non exisiting
    if (mkdir(temp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
    {
        printf("%s ir successfully created", temp);
    }
    else if (errno == EEXIST)
    {
        // mkdir failed
        printf("dir already exists");
    }
    else
    {
        printf("some other error");
    }
    /*time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char s[64];
    assert(strftime(s, sizeof(s), "%c", tm));
    for(int i = 0; i < strlen(s); i++){
        if(s[i]==' '){
            s[i]='_';
        }
    }
    strcat(temp, "/");
    strcat(temp, s);
    strcat(temp, ".txt");
    */
    //create file
    int j = counter(temp);
    if(j == -1)
    {
        printf("Error occured while processing message");
        return false;
    }
    char buf2[BUF];
    snprintf(buf2, sizeof(buf2), "/message%d.txt",j);
    strcat(temp, buf2);
    printf("%s", temp);
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    fd = creat(temp, mode);
    if (fd == -1)
    {
        printf("Error creating file");
        return false;
    }
    close(fd);
    //write msg to file, easier to write to file*
    FILE *f = fopen(temp, "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        return false;
    }
    fprintf(f, "%s%s-\n", user, subject);
    msg* a = head;
    while(a!=NULL)
    {
        fprintf(f, "%s", a->text);
        a = a->next;
    }
    fclose(f);
    
    return true;
}

int counter(char* userdir)
{
    int i;
    char count[BUF];
    char *counter;
    size_t len = 0;
    char numbr[2];
    strcpy(count, userdir);
    strcat(count, "/count");
    FILE *f = fopen(count, "r+"); //open file for reading/writing(doenst have to exist)
    if(f == NULL)
    {
        if((f = fopen(count, "w+")) != NULL) //if file not existing create file to read/write
        {
            i = 1;
        }
        else
        {
            printf("Error creating file");
            return -1;
        }
    }
    else
    {
        // read counter of file
        getline(&counter, &len, f);
        numbr[0] = counter[5];
        numbr[1] = '\0';
        i = (int) strtol(numbr,NULL, 10);
        i++;
        fclose(f);
        // reopen file to overwrite counter
        if((f = fopen(count, "w+")) != NULL)
        {
            fprintf(f, "count%d", i);
        }
        else
        {
            printf("Error opening file");
            return -1;
        }
    }
    fclose(f);
    return i;
}
//TO DO: create(), listen() catch errors
// TODO: create(), listen() catch errors

// TODO check for memory leaks (free...)

// TODO on createfile() it will send OK. no matter if something failed in create file.
// Createfile function is however boolean and it can be additionaly check if something went wrong

// TODO improve performance - data sending is to slow (3-4 nested loops)

// TODO comment code before code review
