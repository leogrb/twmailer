#include "../include/ServerInputHelper.h"
#include "../include/LDAPHandler.h"

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char c, *ptr;
    ptr = vptr;
    for (n = 1; n < maxlen; n++)
    {
    again:
        if ((rc = read(fd, &c, 1)) == 1)
        {
            *ptr++ = c;
            if (c == '\n')
                break; // newline ist stored, like fgets()
        }
        else if (rc == 0)
        {
            if (n == 1)
                return (0); // EOF, no data read
            else
                break; // EOF, some data was read
        }
        else
        {
            if (errno == EINTR)
                goto again;
            return (-1); // error, errno set by read()
        };
    };
    *ptr = 0; // null terminate like fgets()
    return (n);
}

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
    pthread_mutex_lock(&file_lock);
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
    fprintf(f, "%s\n%s-\n", user, subject);
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

long long getMessageID(char *fileName)
{
    char *p = fileName;
    long long id = 0;
    while (*p)
    {
        if (isdigit(*p) || ((*p == '-' || *p == '+') && isdigit(*(p + 1))))
        {
            // Found a number
            id = strtol(p, &p, 10); // Read number
        }
        else
        {
            // Otherwise, move on to the next character.
            p++;
        }
    }
    return id;
}

bool listAllMessages(char *spool, char *user, char *messages)
{
    pthread_mutex_lock(&file_lock);
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
    // path[strlen(path) - 1] = '\0';
    dir = opendir(path);

    if (dir == NULL)
    {
        pthread_mutex_unlock(&file_lock);
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
                    pthread_mutex_unlock(&file_lock);
                    return false;
                }
                getline(&subject, &len, fp); // user name first
                getline(&subject, &len, fp); // second time is subject
                // add message ID
                strcat(messages, "ID: ");
                // convert number from message and parse it
                char *fileName = dp->d_name;
                long long id = getMessageID(fileName);
                char sID[10];
                sprintf(sID, "%lld", id);
                strcat(messages, sID);
                strcat(messages, ", Subject: ");
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
    pthread_mutex_lock(&file_lock);
    char messagePath[BUF];
    char messageName[BUF];

    // user[strlen(user) - 1] = '\0';
    strcpy(messagePath, spool);
    strcat(messagePath, "/");
    strcat(messagePath, user);
    snprintf(messageName, sizeof(messageName), "/message%d.txt", msgNumber);
    strcat(messagePath, messageName);

    FILE *fp;
    if ((fp = fopen(messagePath, "r")) == NULL)
    {
        perror("Cannot open message file");
        pthread_mutex_unlock(&file_lock);
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
    pthread_mutex_lock(&file_lock);
    char temp[BUF];
    char buf2[BUF];
    // user[strlen(user) - 1] = '\0'; //get rid of '\n'
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
        pthread_mutex_unlock(&file_lock);
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
            pthread_mutex_unlock(&file_lock);
            return false;
        }
    }
    return true;
}

void loginFailed(char *buffer, int *new_socket)
{
    printf("Request execution failed. User not logged in.\n");
    strcpy(buffer, "ERR\n");
    send(*new_socket, buffer, strlen(buffer), 0);
}

//thread method
void *handle(void *arg)
{
    int new_socket = *(((thread_struct *)arg)->socket_fd);
    struct sockaddr_in client_address = ((thread_struct *)arg)->client_address;
    char *spool = ((thread_struct *)arg)->spoolpath;
    vector *v = ((thread_struct *)arg)->vec;

    char buffer[BUF];
    char user[10];
    char receiver[10];
    char subject[82];
    char DELnumbr[2];
    int size, valid;
    int numberOfLoginTries = 0;
    bool isLogged = false;
    do
    {
        size = readline(new_socket, buffer, BUF - 1);
        if (size > 0)
        {
            buffer[size] = '\0';
            printf("Message received: %s", buffer);
            if ((strncmp(buffer, "SEND", 4)) == 0 && isLogged)
            {
                printf("Processing SEND request\n");
                send(new_socket, buffer, strlen(buffer), 0);
                char *sendResponse = "SEND\n";
                // process message
                for (int i = 0; i < 3; i++)
                {
                    valid = -1;
                    size = readline(new_socket, buffer, BUF - 1);
                    if (size > 0 && size < 10 && i == 0)
                    {
                        buffer[size] = '\0';
                        strcpy(receiver, buffer);
                        printf("Send to receiver: %s", receiver);
                        valid = 1;
                        send(new_socket, sendResponse, strlen(sendResponse), 0);
                    }
                    else if (size > 0 && size < 82 && i == 1)
                    {
                        buffer[size] = '\0';
                        strcpy(subject, buffer);
                        printf("Mail subject: %s", subject);
                        valid = 1;
                        send(new_socket, sendResponse, strlen(sendResponse), 0);
                    }
                    else if (size > 0 && i == 2)
                    {
                        //linked list for saving msg text
                        buffer[size] = '\0';
                        msg *head = malloc(sizeof(msg));
                        head->next = NULL;
                        strcpy(head->text, buffer);
                        do
                        {
                            size = readline(new_socket, buffer, BUF - 1);
                            if (size > 0)
                            {
                                buffer[size] = '\0';
                                if (size != 2 && buffer[0] != '.' && buffer[1] != '\n')
                                {
                                    push(head, buffer);
                                }
                                valid = 1;
                            }
                            else
                            {
                                freeList(head);
                                valid = -1;
                                break;
                            }
                        } while (size != 2 && buffer[0] != '.' && buffer[1] != '\n');
                        if (valid == 1)
                        {
                            //save msg here
                            if (createmsg(user, receiver, subject, head, spool))
                            {
                                printf("Request successfully executed\n");
                                strcpy(buffer, "OK\n");
                            }
                            else
                            {
                                printf("Request execution failed\n");
                                strcpy(buffer, "ERR\n");
                            }
                            pthread_mutex_unlock(&file_lock);
                            freeList(head);
                            send(new_socket, buffer, strlen(buffer), 0);
                        }
                        buffer[size] = '\0';
                    }
                    else if (valid != 1)
                    {
                        printf("Request execution failed\n");
                        strcpy(buffer, "ERR\n");
                        send(new_socket, buffer, strlen(buffer), 0);
                        break;
                    }
                }
            }
            else if ((strncmp(buffer, "LIST", 4)) == 0 && isLogged)
            {
                printf("Processing LIST request\n");
                send(new_socket, buffer, strlen(buffer), 0);
                // do nothing with readline
                size = readline(new_socket, buffer, BUF - 1);
                bool isValid = false;
                char messages[BUF];
                if (listAllMessages(spool, user, messages))
                {
                    pthread_mutex_unlock(&file_lock);
                    printf("Request successfully executed\n");
                    strcpy(buffer, messages);
                    strcat(buffer, "OK\n");
                    send(new_socket, buffer, strlen(buffer), 0);
                    isValid = true;
                }
                if (!isValid)
                {
                    printf("Request execution failed\n");
                    strcpy(buffer, "ERR\n");
                    send(new_socket, buffer, strlen(buffer), 0);
                }
            }
            else if ((strncmp(buffer, "READ", 4)) == 0 && isLogged)
            {
                printf("Processing READ request\n");
                send(new_socket, buffer, strlen(buffer), 0);
                size = readline(new_socket, buffer, BUF - 1);
                bool isValid = false;
                char output[BUF];
                if (size > 0)
                {
                    int n = 0;
                    int msgNumber;
                    while (n < size && isdigit(buffer[n]))
                    {
                        n++;
                    }
                    if (n == 0)
                    {
                        isValid = false;
                    }
                    if (n == 1)
                    {
                        char readNumber[2];
                        readNumber[0] = buffer[0];
                        readNumber[1] = '\n';
                        msgNumber = (int)strtol(readNumber, NULL, 10);
                        if (msgNumber != 0)
                        {
                            isValid = readMessage(user, msgNumber, spool, output);
                        }
                    }
                    else if (n > 1 && buffer[n] == '\n')
                    {
                        buffer[n] = '\0';
                        msgNumber = (int)strtol(buffer, NULL, 10);
                        isValid = readMessage(user, msgNumber, spool, output);
                    }
                }
                if (!isValid)
                {
                    printf("Request execution failed\n");
                    strcpy(buffer, "ERR\n");
                    send(new_socket, buffer, strlen(buffer), 0);
                }
                else
                {
                    pthread_mutex_unlock(&file_lock);
                    printf("Request successfully executed\n");
                    strcpy(buffer, output);
                    strcat(buffer, "OK\n");
                    send(new_socket, buffer, strlen(buffer), 0);
                }
            }
            else if ((strncmp(buffer, "DEL", 3)) == 0 && isLogged)
            {
                printf("Processing DELETE request\n");
                send(new_socket, buffer, strlen(buffer), 0);
                size = readline(new_socket, buffer, BUF - 1);
                bool DELvalid = false;
                if (size > 0)
                {
                    int n = 0;
                    int msgnr;
                    while (n < size && isdigit(buffer[n]))
                    {
                        n++;
                    }
                    if (n == 0)
                    {
                        DELvalid = false;
                    }
                    if (n == 1)
                    {
                        DELnumbr[0] = buffer[0];
                        DELnumbr[1] = '\0';
                        msgnr = (int)strtol(DELnumbr, NULL, 10);
                        if (msgnr != 0)
                        {
                            DELvalid = deletemsg(user, msgnr, spool);
                        }
                    }
                    else if (n > 1 && buffer[n] == '\n')
                    {
                        buffer[n] = '\0';
                        msgnr = (int)strtol(buffer, NULL, 10);
                        DELvalid = deletemsg(user, msgnr, spool);
                    }
                }
                if (!DELvalid)
                {
                    printf("Request execution failed\n");
                    strcpy(buffer, "ERR\n");
                    send(new_socket, buffer, strlen(buffer), 0);
                }
                else
                {
                    pthread_mutex_unlock(&file_lock);
                    printf("Request successfully executed\n");
                    strcpy(buffer, "OK\n");
                    send(new_socket, buffer, strlen(buffer), 0);
                }
            }
            else if ((strncmp(buffer, "LOGIN", 5)) == 0 && !isLogged)
            {
                printf("Processing LOGIN request\n");
                send(new_socket, buffer, strlen(buffer), 0);
                char *loginResponse = "LOGIN\n";
                char password[BUF];
                for (int i = 0; i < 2; i++)
                {
                    size = readline(new_socket, buffer, BUF - 1);
                    if (i == 0)
                    {
                        if (size > 0 && size < 10)
                        {
                            buffer[size] = '\0';
                            strcpy(user, buffer);
                            send(new_socket, loginResponse, strlen(loginResponse), 0);
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        buffer[size] = '\0';
                        strcpy(password, buffer);
                        isLogged = userLogin(user, password);
                    }
                }
                if (isLogged)
                {
                    printf("Request successfully executed\n");
                    strcpy(buffer, "OK\n");
                    send(new_socket, buffer, strlen(buffer), 0);
                }
                else
                {
                    numberOfLoginTries++;
                    if (numberOfLoginTries >= 3)
                    {
                        strcpy(buffer, "quit\n");
                        send(new_socket, buffer, strlen(buffer), 0);
                        printf("Request execution failed\n");
                        printf("Client closed remote socket - IP Address has been blocked for 30 minutes\n");
                        ip_t ipAddress;
                        ipAddress.ip_address = inet_ntoa(client_address.sin_addr);
                        ipAddress.saved_time = time(NULL);
                        vector_add(v, &ipAddress);
                        close(new_socket);
                        pthread_exit(NULL);
                    }
                    printf("Request execution failed\n");
                    strcpy(buffer, "ERR\n");
                    send(new_socket, buffer, strlen(buffer), 0);
                }
            }
            else
            {
                if (strncmp(buffer, "SEND", 4) == 0 ||
                    strncmp(buffer, "LIST", 4) == 0 ||
                    strncmp(buffer, "READ", 4) == 0 ||
                    strncmp(buffer, "DEL", 3) == 0 ||
                    (strncmp(buffer, "LOGIN", 5) == 0 && isLogged))
                {
                    loginFailed(buffer, &new_socket);
                }
                else if (strncmp(buffer, "quit", 4) != 0)
                {
                    // handling all other received messages
                    send(new_socket, buffer, strlen(buffer), 0);
                }
            }
        }
        else if (size == 0)
        {
            printf("Client closed remote socket\n");
            break;
        }
        else
        {
            perror("recv error");
        }
    } while (strncmp(buffer, "quit", 4) != 0);
    close(new_socket);
    pthread_exit(NULL);
}

bool isAddressBlocked(vector *v, ip_t *ip)
{
    printf("Count: %d\n", vector_count(v));
    // first get index of address from vector
    int index = vector_get_index(v, ip->ip_address);
    printf("Index: %d\n", index);
    if (index < 0)
    {
        return false;
    }
    ip_t *saved_ip = vector_get(v, index);
    time_t cur_time = time(NULL);
    time_t saved_time = saved_ip->saved_time;
    printf("Diff: %ld\n", cur_time - saved_time);
    if (cur_time - saved_time < MIN_30)
    {
        return true;
    }
    // if 30 min expired already, remove address from vector
    vector_delete(v, index);
    return false;
}