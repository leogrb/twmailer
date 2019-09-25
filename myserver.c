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
#define BUF 1024
typedef struct node{
    char text[BUF];
    struct node* next;
}msg;
void push(msg *head, char msgtext[]);
void freeList(msg *head);
bool createfile(char* user, char* receiver, char* subject, msg* head, char* spool);
ssize_t readline(int fd, void *vptr, size_t maxlen);

int main (int argc, char **argv) {
    int create_socket, new_socket;
    socklen_t addrlen;
    char buffer[BUF];
    char user[10];
    char receiver[10];
    char subject[82];
    int size;
    int valid;
    struct sockaddr_in address, cliaddress;
    if(argc < 3) {
        printf("Usage: %s port spooldirectory\n", argv[0]);
        return EXIT_FAILURE;
    }
    int port = atoi(argv[1]);
    printf("%d", port);
    char* spool = (char*)malloc(sizeof(char) * strlen(argv[2]));
    spool = argv[2];
    create_socket = socket (AF_INET, SOCK_STREAM, 0);

    memset(&address,0,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons (port);

    if (bind ( create_socket, (struct sockaddr *) &address, sizeof (address)) != 0) {
        perror("bind error");
        return EXIT_FAILURE;
    }
    listen (create_socket, 5);

    addrlen = sizeof (struct sockaddr_in);
    
    //Create spool directory if non exisiting
    if(mkdir(spool, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0){
        printf("%s ir successfully created", spool);
    }
    else if(errno == EEXIST){
        // mkdir failed
        printf("dir already exists");
    }
    else{
        printf("some other error");
    }
    while (1) {
        printf("Waiting for connections...\n");
        new_socket = accept ( create_socket, (struct sockaddr *) &cliaddress, &addrlen );
        if (new_socket > 0) {
            printf ("Client connected from %s:%d...\n", inet_ntoa (cliaddress.sin_addr),ntohs(cliaddress.sin_port));
            strcpy(buffer,"Welcome to myserver, Please enter your command:\n");
            send(new_socket, buffer, strlen(buffer),0);
        }
        do {
            size = readline(new_socket, buffer, BUF-1);
            if( size > 0) {
                buffer[size] = '\0';
                printf("%d", size);
                printf ("Message received: %s", buffer);
                if((strncmp(buffer, "SEND", 4)) == 0){
                   // process message
                     for(int i = 0; i < 4; i++){
                            printf("%d", i);
                            valid = -1;
                        size = readline(new_socket, buffer, BUF-1);
                        if( size > 0 && size < 10 && i < 2) {
                            buffer[size] = '\0';
                            if(i==0){
                                strcpy(user, buffer);
                                printf("user: %s", user);
                                valid=1;
                            } else if(i == 1){
                                strcpy(receiver, buffer);
                                printf("receiver: %s", receiver);
                                valid=1;
                            }
                        }
                        else if(size > 0 && size < 82 && i == 2){
                            buffer[size] = '\0';
                            strcpy(subject, buffer);
                            printf("subject: %s", subject);
                            valid = 1;
                        }
                        else if(size > 0 && i == 3){
                            //linked list for saving msg text
                            buffer[size] = '\0';
                            msg *head = malloc(sizeof(msg));
                            head->next = NULL;
                            strcpy(head->text, buffer);
                            do{
                                // fix: if msg is empty (just '\n'), '\n.' is still stored
                                size = readline(new_socket, buffer, BUF-1);
                                if(size > 0){
                                    buffer[size] = '\0';
                                    push(head, buffer);
                                    valid = 1;
                                }
                                else {
                                    freeList(head);
                                    valid = -1;
                                    break;
                                }
                            } while(size != 2 && buffer[0] != '.' && buffer[1] != '\n');
                            if(valid == 1){
                                printf("%s", head->text);
                                printf("%s", head->next->text);
                                //save msg here
                                createfile(user, receiver, subject, head, spool);
                                freeList(head);
                                strcpy(buffer, "OK\n");
                                send(new_socket, buffer, strlen(buffer),0);
                            }
                            buffer[size] = '\0';
                        }
                        else if(valid != 1){
                            printf("%d", i);
                            strcpy(buffer, "ERR\n");
                            send(new_socket, buffer, strlen(buffer),0);
                            break;
                        }
                    }
                }

            } else if (size == 0) {
                printf("Client closed remote socket\n");
                break;
            } else {
                perror("recv error");
                return EXIT_FAILURE;
            }
        } while (strncmp (buffer, "quit", 4)  != 0);
        close (new_socket);
    }
    close (create_socket);
    return EXIT_SUCCESS;
}

ssize_t readline(int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc ;
    char c, *ptr ;
    ptr = vptr ;
    for (n = 1 ; n < maxlen ; n++) {
        again:
        if ( (rc = read(fd,&c,1)) == 1) {
            *ptr++ = c ;
            if (c == '\n')
                break ; // newline ist stored, like fgets()
        } else if (rc == 0) {
            if (n == 1)
                return (0) ; // EOF, no data read
            else
                break ; // EOF, some data was read
        } else {
            if (errno == EINTR)
                goto again ;
            return (-1) ; // error, errno set by read()
        } ;
    } ;
    *ptr = 0 ; // null terminate like fgets()
    return (n) ;
}
void push(msg *head, char msgtext[]){
    msg *current = head;
    if(head->text[0]!='\0'){
        while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(msg));
    strcpy(current->next->text, msgtext);
    current->next->next = NULL;
    }
    else{
        strcpy(head->text, msgtext);
    }
}

void freeList(msg *head)
{
   msg* tmp;
   while (head != NULL)
    {
       tmp = head;
       head = head->next;
       free(tmp);
    }

}


bool createfile(char* user, char* receiver, char* subject, msg* head, char* spool){
    int fd, j;
    char temp[BUF];
    strcpy(temp, spool);
    //Create user directory if non exisiting
    // to do: build receiver folder with spool
    if(mkdir(receiver, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0){
        printf("%s ir successfully created", receiver);
    }
    else if(errno == EEXIST){
        // mkdir failed
        printf("dir already exists");
    }
    else{
        printf("some other error");
    }
    return true;
}

//TO DO: create(), listen() catch errors
