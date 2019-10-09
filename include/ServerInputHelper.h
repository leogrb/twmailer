/**
 * Helper header file for input options
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>

#include "message.h"
#define BUF 1024

int counter(char *userdir)
{
    int i;
    char count[BUF];
    char *counter;

    size_t len = 0;
    char numbr[2];
    strcpy(count, userdir);
    strcat(count, "/count");
    FILE *f = fopen(count, "r+"); //open file for reading/writing(doenst have to exist)
    if (f == NULL)
    {
        if ((f = fopen(count, "w+")) != NULL) //if file not existing create file to read/write
        {
            i = 1;
            fprintf(f, "count%d", i);
        }
        else
        {
            perror("Error creating file");
            return -1;
        }
    }
    else
    {
        // read counter of file
        getline(&counter, &len, f);
        numbr[0] = counter[5];
        numbr[1] = '\0';
        i = (int)strtol(numbr, NULL, 10);
        i++;
        fclose(f);
        // reopen file to overwrite counter
        if ((f = fopen(count, "w+")) != NULL)
        {
            fprintf(f, "count%d", i);
        }
        else
        {
            perror("Error opening file");
            return -1;
        }
    }
    fclose(f);
    return i;
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
        //printf("%s ir successfully created", temp);
    }
    else if (errno == EEXIST)
    {
        // mkdir failed
        //printf("dir already exists");
    }
    else
    {
        //printf("some other error");
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
    if (j == -1)
    {
        perror("Error occured while processing message");
        return false;
    }
    char buf2[BUF];
    snprintf(buf2, sizeof(buf2), "/message%d.txt", j);
    strcat(temp, buf2);
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    fd = creat(temp, mode);
    if (fd == -1)
    {
        perror("Error creating file");
        return false;
    }
    close(fd);
    //write msg to file, easier to write to file*
    FILE *f = fopen(temp, "w");
    if (f == NULL)
    {
        perror("Error opening file!\n");
        return false;
    }
    fprintf(f, "%s%s-\n", user, subject);
    msg *a = head;
    while (a != NULL)
    {
        fprintf(f, "%s", a->text);
        a = a->next;
    }
    fclose(f);

    return true;
}

int getNumberOfMessages(char *dirPath)
{
    // on list check also for number of saved messages in directory
    int fileCounter = 0;
    DIR *dirp;
    struct dirent *entry;
    dirp = opendir(dirPath);
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_type == DT_REG && strncmp(entry->d_name, "count", 5) != 0)
        {
            fileCounter++;
        }
    }
    closedir(dirp);
    return fileCounter;
}

bool listAllMessages(char *spool, char *user, char *messages)
{
    struct dirent *dp;
    DIR *dir;
    FILE *fp;
    char path[BUF];
    char *subject;
    size_t len = 0;

    strcpy(path, spool);
    strcat(path, "/");
    strcat(path, user);
    // get rid of new line char at the end of the string
    path[strlen(path) - 1] = '\0';
    dir = opendir(path);

    if (dir == NULL)
    {
        perror("Cannot open directory");
        return false;
    }
    int numberOfMessages = getNumberOfMessages(path);
    sprintf(messages, "Number of messages: %d", numberOfMessages);
    strcat(messages, "\n");
    while ((dp = readdir(dir)) != NULL)
    {
        if (!(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0))
        {
            // check for regular files and skip count file
            if (dp->d_type == DT_REG && strncmp(dp->d_name, "count", 5) != 0)
            {
                char msgPath[BUF];
                strcpy(msgPath, path);
                strcat(msgPath, "/");
                strcat(msgPath, dp->d_name);
                if ((fp = fopen(msgPath, "r")) == NULL)
                {
                    perror("Cannot open message file");
                    return false;
                }
                getline(&subject, &len, fp);           // user name first
                getline(&subject, &len, fp);           // second time is subject
                // add message ID
                strcat(messages, "(");
                strcat(messages, dp->d_name);
                strcat(messages, "): ");
                // add subject to output line
                strcat(messages, subject);
                fclose(fp);
            }
        }
    }
    closedir(dir);
    return true;
}

bool readMessage(char *user, int msgNumber, char *spool, char *output)
{
    char messagePath[BUF];
    char messageName[BUF];

    user[strlen(user) - 1] = '\0';
    strcpy(messagePath, spool);
    strcat(messagePath, "/");
    strcat(messagePath, user);
    snprintf(messageName, sizeof(messageName), "/message%d.txt", msgNumber);
    strcat(messagePath, messageName);

    FILE *fp;
    if ((fp = fopen(messagePath, "r")) == NULL)
    {
        perror("Cannot open message file");
        return false;
    }
    fseek(fp, 0, SEEK_END);
    size_t fileLength = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    output[fileLength] = '\0';
    fread(output, fileLength, 1, fp);
    fclose(fp);
    return true;
}

bool deletemsg(char *user, int msgid, char *spool)
{
    char temp[BUF];
    char buf2[BUF];
    user[strlen(user) - 1] = '\0'; //get rid of '\n'
    //build folder for user
    strcpy(temp, spool);
    strcat(temp, "/");
    strcat(temp, user);
    snprintf(buf2, sizeof(buf2), "/message%d.txt", msgid);
    strcat(temp, buf2);
    FILE *f = fopen(temp, "r+"); //open file for reading/writing(doenst have to exist)
    if (f == NULL)
    {
        perror("file doesnt exist");
        return false;
    }
    else
    {
        fclose(f);
        int status = remove(temp);
        if (status == -1)
        {
            if (errno == EBUSY)
            {
                perror("File is used by other process");
            }
            return false;
        }
    }
    return true;
}
