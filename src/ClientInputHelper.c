#include "../include/ClientInputHelper.h"

void maskpass(char *pwd)
{
    // static struct termios oldt, newt;
    // int i = 0;
    // int c;

    // /*saving the old settings of STDIN_FILENO and copy settings for resetting*/
    // tcgetattr(STDIN_FILENO, &oldt);
    // newt = oldt;

    // /*setting the approriate bit in the termios struct*/
    // newt.c_lflag &= ~(ECHO);

    // /*setting the new bits*/
    // tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // /*reading the password from the console*/
    // while ((c = getchar()) != '\n' && c != EOF && i < BUF)
    // {
    //     pwd[i++] = c;
    // }
    // pwd[i] = '\0';
    // /*resetting our old STDIN_FILENO*/
    // tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

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

void handleLogin(char *buffer, int *create_socket)
{
    for (int i = 0; i < COMMAND_LOGIN_OPT; i++)
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
            printf("Password: ");
            // maskpass(buffer);
            fgets(buffer, BUF, stdin);
            if (!sendReceive(buffer, create_socket))
            {
                break;
            }
        }
    }
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
    else if (strcmp(buffer, "LOGIN\n") == 0)
    {
        handleLogin(buffer, create_socket);
    }
}
