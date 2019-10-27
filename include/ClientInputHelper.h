#ifndef CLIENT_INPUT_HELPER_H
#define CLIENT_INPUT_HELPER_H

#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>

#define BUF 1024

#define COMMAND_SEND_OPT 3
#define COMMAND_LOGIN_OPT 2

bool sendReceive(char *buffer, int *create_socket);

void handleSend(char *buffer, int *create_socket);

void handleRead(char *buffer, int *create_socket);

void handleList(char *buffer, int *create_socket);

void handleDel(char *buffer, int *create_socket);

void handleLogin(char *buffer, int *create_socket);

void commandHandler(char *buffer, int *create_socket);

void maskpass(char *pwd);

#endif