/**
 * Helper header file for input options
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>

#define BUF 1024

int getNumberOfMessages(char *dirPath)
{
    int fileCounter = 0;
    DIR *dirp;
    struct dirent *entry;
    dirp = opendir(dirPath);
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            fileCounter++;
        }
    }
    closedir(dirp);
    return fileCounter;
}

bool listAllMessages(char *spool, char *user)
{
    struct dirent *dp;
    DIR *dir;
    FILE *fp;
    char messages[BUF];
    char path[BUF];
    char *subject;
    size_t len = 0;

    strcpy(path, spool);
    strcat(path, "/");
    strcat(path, user);

    path[strlen(path) - 1] = '\0';
    dir = opendir(path);

    if (dir == NULL)
    {
        perror("Cannot open directory");
        return false;
    }
    int numberOfMessages = getNumberOfMessages(path);
    sprintf(messages, "%d", numberOfMessages);
    strcat(messages, "\n");
    while ((dp = readdir(dir)) != NULL)
    {
        if (!(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0))
        {
            if (dp->d_type == DT_REG) // for regular file
            {
                char msgPath[BUF];
                strcpy(msgPath, path);
                strcat(msgPath, "/");
                strcat(msgPath, dp->d_name);
                printf("%s\n", msgPath);
                if ((fp = fopen(msgPath, "r")) == NULL)
                {
                    perror("Cannot open message file");
                    return false;
                }
                getline(&subject, &len, fp); // user name first
                getline(&subject, &len, fp); // second time is subject
                strcat(messages, subject);
                // strcat(messages, "\n");
                fclose(fp);
            }
        }
    }
    closedir(dir);
    printf("%s", messages);
    return true;
}