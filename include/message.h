/*
header file for linked list implementation used for storing multi-line messages
*/

#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF 1024

typedef struct node
{
    char text[BUF];
    struct node *next;
} msg;

void push(msg *head, char msgtext[]);

void freeList(msg *head);

#endif
