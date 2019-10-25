#include "../include/message.h"

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