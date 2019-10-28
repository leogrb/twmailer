#ifndef SERVER_INPUT_HELPER_H
#define SERVER_INPUT_HELPER_H
/**
 * Helper header file for input options
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#include "message.h"
#include "thread.h"
#include "vector.h"
#include "ip.h"

#define BUF 1024

pthread_mutex_t file_lock;

ssize_t readline(int fd, void *vptr, size_t maxlen);

int counter(char *userdir);

bool createmsg(char *user, char *receiver, char *subject, msg *head, char *spool);

int getNumberOfMessages(char *dirPath);

long long getMessageID(char *fileName);

bool listAllMessages(char *spool, char *user, char *messages);

bool readMessage(char *user, int msgNumber, char *spool, char *output);

bool deletemsg(char *user, int msgid, char *spool);

//thread method
void *handle(void *arg);

bool isAddressBlocked(vector *v, ip_t *ip);

#endif