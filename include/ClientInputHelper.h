#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define BUF 1024

#define COMMAND_SEND_OPT 4
#define COMMAND_READ_OPT 2
#define COMMAND_DEL_OPT 2

bool sendReceive(char *buffer, int *create_socket)
{
    int size;
    send(*create_socket, buffer, strlen(buffer), 0);
    size = recv(*create_socket, buffer, BUF - 1, 0);

    if (size > 0)
    {
        buffer[size] = '\0';
        if (strncmp(buffer, "ERR", 3) == 0 || strncmp(buffer, "OK", 2) == 0)
        {
            // Only print OK and ERR
            printf("%s", buffer);
        }
        if (strncmp(buffer, "ERR", 3) == 0)
        {
            return false;
        }
    }
    return true;
}

void handleSend(char *buffer, int *create_socket)
{
    printf("---- SEND ----\n");
    for (int i = 0; i < COMMAND_SEND_OPT; i++)
    {
        if (i == 0)
        {
            printf("Sender: ");
            fgets(buffer, BUF, stdin);
            if (!sendReceive(buffer, create_socket))
            {
                break;
            }
        }
        else if (i == 1)
        {
            printf("Receiver: ");
            fgets(buffer, BUF, stdin);
            if (!sendReceive(buffer, create_socket))
            {
                break;
            }
        }
        else if (i == 2)
        {
            printf("Subject: ");
            fgets(buffer, BUF, stdin);
            if (!sendReceive(buffer, create_socket))
            {
                break;
            }
        }
        else
        {
            printf("Message (new line and . (fullstop) will break message input!):\n");
            while (true)
            {
                fgets(buffer, BUF, stdin);
                send(*create_socket, buffer, strlen(buffer), 0);
                if (strcmp(buffer, ".\n") == 0)
                {
                    break;
                }
            }
            int size = recv(*create_socket, buffer, BUF - 1, 0);
            if (size > 0)
            {
                buffer[size] = '\0';
                printf("%s", buffer);
            }
        }
    }
    printf("--------------\n");
}

void handleRead(char *buffer, int *create_socket)
{
    printf("---- READ ----\n");
    for (int i = 0; i < COMMAND_READ_OPT; i++)
    {
        if (i == 0)
        {
            printf("Username: ");
            fgets(buffer, BUF, stdin);
            if (!sendReceive(buffer, create_socket))
            {
                break;
            }
        }
        else
        {
            printf("Message ID: ");
            fgets(buffer, BUF, stdin);
            if (!sendReceive(buffer, create_socket))
            {
                break;
            }
            printf("%s", buffer);
        }
    }
    printf("--------------\n");
}

void handleList(char *buffer, int *create_socket)
{
    printf("---- LIST ----\n");
    printf("Username: ");
    fgets(buffer, BUF, stdin);
    if (sendReceive(buffer, create_socket))
    {
        printf("%s", buffer);
    }
    printf("--------------\n");
}

void handleDel(char *buffer, int *create_socket)
{
    printf("---- DEL ----\n");
    for (int i = 0; i < COMMAND_DEL_OPT; i++)
    {
        if (i == 0)
        {
            printf("Username: ");
            fgets(buffer, BUF, stdin);
            if (!sendReceive(buffer, create_socket))
            {
                break;
            }
        }
        else
        {
            printf("Message ID: ");
            fgets(buffer, BUF, stdin);
            if (!sendReceive(buffer, create_socket))
            {
                break;
            }
        }
    }
    printf("-------------\n");
}

void commandHandler(char *buffer, int *create_socket)
{
    if (!sendReceive(buffer, create_socket))
    {
        return;
    }
    if (strcmp(buffer, "SEND\n") == 0)
    {
        handleSend(buffer, create_socket);
    }
    else if (strcmp(buffer, "READ\n") == 0)
    {
        handleRead(buffer, create_socket);
    }
    else if (strcmp(buffer, "LIST\n") == 0)
    {
        handleList(buffer, create_socket);
    }
    else if (strcmp(buffer, "DEL\n") == 0)
    {
        handleDel(buffer, create_socket);
    }
}