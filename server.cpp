#ifndef unix
#define WIN32
#include <windows.h>
#include <winsock.h>
#else
#define closesocket close
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <new>
#define PROTOPORT 5193 
#define QLEN 6        
using namespace std;
int visits = 0;

/********************************************************************
*Function countChars. Counts the number of characters in the file.
********************************************************************/
int countChars(FILE *file, char* fileName) {

    file = fopen(fileName, "r");
    char ch;
    int i = 0;

    if (file == NULL)
        return 0;


    while ((ch = fgetc(file)) != EOF) {
        i++;
    }

    return i;
} 


/********************************************************************
*Function SendFileOverSocket
********************************************************************/
int SendFileOverSocket(int sd, char *fileName, int file_size) {
    
    int file_desc;
    char *data;

    printf("Sending File...\n");

    // Open the file
    FILE *fp = fopen(fileName, "r");

    file_size = countChars(fp, fileName);
    write(sd, &file_size, sizeof(int));

    // Send the file
    
    data = static_cast<char*>(malloc(file_size + 1));
    fread(data, file_size, file_size, fp);
    send(sd, data, file_size, 0);

    printf("File %s sent\n", fileName);
    return 1;
}

/********************************************************************
*Function doUpload
********************************************************************/
void doUpload(char *fileName, int socket) {

    int recvFile;
    printf("Performing Upload request of client\n");
    char server_response[BUFSIZ], client_response[BUFSIZ];

    // Send an acknowledgement message
    strcpy(server_response, "OK");
    write(socket, server_response, strlen(server_response));

    int file_size;
    char *data;

    recv(socket, &file_size, sizeof(int), 0);
    data = static_cast<char*>(malloc(file_size + 1));

    // Create new file to receive and store data
    FILE *fp = fopen(fileName, "w");
    recvFile = recv(socket, data, file_size, 0);

    data[recvFile] = '\0';

    cout << "Size of file recieved is: " << recvFile;
    recvFile = fputs(data, fp);

    fclose(fp);
}

/********************************************************************
*Function doDownload 
********************************************************************/
void doDownload(char *fileName, int socket, int fileSize) {

    char serverResponse[BUFSIZ];

    cout << "Performing Download request of client" << endl;

    strcpy(serverResponse, "OK");
    write(socket, serverResponse, strlen(serverResponse));

    SendFileOverSocket(socket, fileName, fileSize);
}

/********************************************************************
*Function doDelete
********************************************************************/
void doDelete(char *fileName, int socket) {

    char serverResponse[BUFSIZ];
    cout << "Performing Delete request of client\n";
    
    strcpy(serverResponse, "OK");
    write(socket, serverResponse, strlen(serverResponse));
     
    remove(fileName);
}

/********************************************************************
*Function doRename 
********************************************************************/
void doRename(char *oldFileName, char *newFileName, int socket) {

    char serverResponse[BUFSIZ];

    cout << "Performing Rename request of client...";

    strcpy(serverResponse, "OK");
    write(socket, serverResponse, strlen(serverResponse));
    
    rename(oldFileName, newFileName);
}

/********************************************************************
*Function GetCommandFromRequest. 
********************************************************************/
int GetCommandFromRequest(char *request) {

    char command[10000];
    strcpy(command, request);

    int i = 0;
    
    while (request[i] != ' ' && request[i] != '\0')
        i++;

    if (request[i] == '\0')
        return 5;
    
    else {
        strncpy(command, request, i - 1);
        command[i] = '\0';
    }

    if (!strcmp(command, "DOWNLOAD"))
        return 1;
    else if (!strcmp(command, "UPLOAD"))
        return 2;
    else if (!strcmp(command, "DELETE"))
        return 3;
    else if (!strcmp(command, "RENAME"))
        return 4;
    else if (!strcmp(command, "EXIT"))
        return 5;
    return 0;
}

/********************************************************************
*Function GetArgumentFromRequest. 
********************************************************************/
char* GetArgumentFromRequest(char *request) {
    char *arg = strchr(request, ' ');
    return arg + 1;
}

/********************************************************************
*Function Get3ArgumentsFromRequest. 
********************************************************************/
char** Get3ArgumentsFromRequest(char *request) {
    
    char** array = new char*[3];
    int i = 0;

    array[i++] = strtok(request, " ");

    while (i < 3)
    {
        array[i++] = strtok(NULL, " ");
    }
    
    return array;
}


/********************************************************************
*Function CommandProcessor. Processes the command input by the user.*
********************************************************************/
void *CommandProcessor(void *sd) {

int choice, fileDescrptr, fileSize, socket;
char reply[BUFSIZ], fileExt[BUFSIZ], clientRequest[BUFSIZ], 
    fileName[BUFSIZ], fileNewName[BUFSIZ];
char **strings;
char *data;
socket = *(int *)sd;

while (1) {

    cout << "\nAwaiting command\n";
    int l = recv(socket, clientRequest, BUFSIZ, 0); /*Receive*/

    clientRequest[l] = '\0';
    cout << "Command Received: ----- " << clientRequest << endl;
    choice = GetCommandFromRequest(clientRequest);
    switch (choice) {
        
        case 1:
            strcpy(fileName, GetArgumentFromRequest(clientRequest));
            doDownload(fileName, socket, fileSize);
            break;

        case 2:
            strcpy(fileName, GetArgumentFromRequest(clientRequest));
            doUpload(fileName, socket);
            break;

        case 3:
            strcpy(fileName, GetArgumentFromRequest(clientRequest));
            doDelete(fileName, socket);
            break;
        
        case 4:
            strings = Get3ArgumentsFromRequest(clientRequest);
            strcpy(fileName, strings[1]);
            strcpy(fileNewName, strings[2]);
            doRename(fileName, fileNewName, socket);
            delete[] strings;
            break;
        case 5:
            pthread_exit(0);
            free(sd);
            return 0;
        default:
            break;
    }
}
    pthread_exit(0);
    free(sd);
    return 0;
}

/********************************************************************
*Function main. 
Handles the main server loop that keeps the server application 
running for the the clients.
********************************************************************/

int main(int argc, char *argv[]) {

    cout << "Host FTP server is running..." << endl; 
    struct hostent *ptrh;               /* pointer to a host table entry */
    struct protoent *ptrp;              /* pointer to a protocol table entry */
    struct sockaddr_in sad;             /* structure to hold server.s address */
    struct sockaddr_in cad;             /* structure to hold client.s address */
    unsigned int sd, sd2;               /* socket descriptors */
    int port;                           /* protocol port number */
    int alen;                           /* length of address */
    char buf[1000];                     /* buffer for string the server sends */
   
    memset((char *)&sad, 0, sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET;             /* set family to Internet */
    sad.sin_addr.s_addr = INADDR_ANY;     /* set the local IP address */

    /* Check command-line argument for protocol port and extract */
    /* port number if one is specified. Otherwise, use the default */
    /* port value given by constant PROTOPORT */
    
    if (argc > 1) { /* if argument specified */
        port = atoi(argv[1]); /* convert argument to binary */
    }
    else {
        port = PROTOPORT; /* use default port number */
    }

    if (port > 0) /* test for illegal value */
        sad.sin_port = htons((u_short)port);
    else { 
        /* print error message and exit */
        fprintf(stderr, "bad port number %s\n", argv[1]);
        exit(1);
    }
    /* Map TCP transport protocol name to protocol number */
    if ( ((int*)(ptrp = getprotobyname("tcp"))) == 0) {
        fprintf(stderr, "cannot map \"tcp\" to protocol number");
        exit(1);
    }
    /* Create a socket */
    sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd < 0) {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }

    /* Bind a local address to the socket */
    if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0){
        fprintf(stderr, "bind failed\n");
        cout << errno << endl;
        exit(1);
    } 
    /* Specify size of request queue */
    if (listen(sd, QLEN) < 0) {
        fprintf(stderr, "listen failed\n");
        exit(1);
    }

    pthread_t threads[200]; /* To hold maximum number of threads */
    int i = 0;              // Counter variable

    /* Main server loop - accept and handle requests */
    char dn[BUFSIZ];
    int c = sizeof(struct sockaddr_in);


    while (sd2 = accept(sd, (struct sockaddr *)&cad, (socklen_t*)&c)) {

        // For the multithreaded architecture
        send(sd2, buf, strlen(buf), 0);
        pthread_create(&threads[i++], NULL, CommandProcessor, &sd2);
        pthread_join(threads[i], NULL);
    }
}

