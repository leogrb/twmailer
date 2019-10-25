#ifndef CLIENT_INPUT_HELPER_H
#define CLIENT_INPUT_HELPER_H

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

bool sendReceive(char *buffer, int *create_socket);

void handleSend(char *buffer, int *create_socket);

void handleRead(char *buffer, int *create_socket);

void handleList(char *buffer, int *create_socket);

void handleDel(char *buffer, int *create_socket);

void commandHandler(char *buffer, int *create_socket);

#endif