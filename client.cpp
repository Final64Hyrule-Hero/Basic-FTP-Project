#include <pthread.h>
#include <iostream>
#include <fstream>
#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define PROTOPORT 5193 /* default protocol port number */
using namespace std;

char localhost[] = "localhost"; /* default host name */

/********************************************************************
*Function sendFileOverSocket
********************************************************************/
int SendFileOverSocket(int sd, char *fileName, int file_size) {
    
    int file_desc;
    char* data;

    printf("Sending File...\n");

    // Open the file
    FILE *fp = fopen(fileName, "r");
    // write(sd, &file_size, sizeof(int));

    // Send file size to the server 
    send(sd, &file_size, sizeof(int), 0);

    // Send the file
    data = static_cast<char*>(malloc(file_size + 1));
    fread(data, file_size, file_size, fp);

    send(sd, data, file_size, 0);

    printf("File %s sent\n", fileName);
    return 1;
}

/********************************************************************
*Function doDownload
********************************************************************/
void doDownload(char *fileName, int sd) {

    char RequestMesg[BUFSIZ], ReplyMesg[BUFSIZ];
    int fileSize;
    char *data;
    int t;    
    int recvFile;
    
    // Get a file from server
    strcpy(RequestMesg, "DOWNLOAD ");
    strcat(RequestMesg, fileName);
    write(sd, RequestMesg, strlen(RequestMesg));
    t = recv(sd, ReplyMesg, 2, 0);
    ReplyMesg[t] = '\0';

    printf("%s\n", ReplyMesg);

    // Send an acknowledgement message
    strcpy(ReplyMesg, "OK");

    recv(sd, &fileSize, sizeof(int), 0);
    data = static_cast<char*>(malloc(fileSize + 1));

    FILE *fp = fopen(fileName, "w");
    recvFile = recv(sd, data, fileSize, 0);
    data[recvFile] = '\0';

    recvFile = fputs(data, fp);

    fclose(fp);

    if (strcmp(ReplyMesg, "OK") == 0)
        cout << "File Downloaded!" << endl;
    else
        cout << "Server cannot find file for client...";
}

/********************************************************************
*Function doUpload
********************************************************************/
void doUpload(char *fileName, int sd) {

    char RequestMesg[BUFSIZ], ReplyMesg[BUFSIZ];
    int fileSize;
    char *data;
    int t;    

    strcpy(RequestMesg, "UPLOAD");
    strcat(RequestMesg, " ");
    strcat(RequestMesg, fileName);
    write(sd, RequestMesg, strlen(RequestMesg));
    t = recv(sd, ReplyMesg, BUFSIZ, 0);
    ReplyMesg[t] = '\0';

    if (strcmp(ReplyMesg, "OK") == 0)
        SendFileOverSocket(sd, fileName, 100);
    else
        cout << "Server cannot create file...";
}

/********************************************************************
*Function doDelete
********************************************************************/
void doDelete(char *fileName, int sd) {

    char RequestMesg[BUFSIZ], ReplyMesg[BUFSIZ];
    int file_size;
    char *data;
    int t;

    // Get a file from server
    strcpy(RequestMesg, "DELETE ");
    strcat(RequestMesg, fileName);
    write(sd, RequestMesg, strlen(RequestMesg));
    t = recv(sd, ReplyMesg, 2, 0);
    ReplyMesg[t] = '\0';
    printf("%s\n", ReplyMesg);

    if (strcmp(ReplyMesg, "OK") == 0)  {
        // If File is present at server, start receiving the file and storing locally
        cout << "File deleted\n";
    }
    else {
        cout << "File doesn't exist at server. Aborting. \n";
    }
}

/********************************************************************
*Function doRename
********************************************************************/
void doRename(char *fileName, char *newFileName, int sd) {

    char RequestMesg[BUFSIZ], ReplyMesg[BUFSIZ];
    int fileSize;
    char *data;
    int t;

    /*Get file from the server*/
    strcpy(RequestMesg, "RENAME ");
    strcat(RequestMesg, fileName);
    strcat(RequestMesg, " ");
    strcat(RequestMesg, newFileName);
    write(sd, RequestMesg, strlen(RequestMesg));
    recv(sd, ReplyMesg, 2, 0);
    ReplyMesg[2] = '\0';
    printf("%s\n", ReplyMesg);

    /* Test if file exists to Rename.*/
    if (strcmp(ReplyMesg, "OK") == 0)     
        cout << "File successfully RENAMED!";
    else 
        cout << "File doesn't exist at server. ABORTING.\n";
}


/*------------------------------------------------------------------------
 * * * Program: client
 * * *
 * * * Purpose: allocate a socket, connect to a server, and print all output
 * * * Syntax: client [ host [port] ]
 * * *
 * * * host - name of a computer on which server is executing
 * * * port - protocol port number server is using
 * * *
 * * * Note: Both arguments are optional. If no host name is specified,
 * * * the client uses "localhost"; if no protocol port is
 * * * specified, the client uses the default given by PROTOPORT.
 * * *
 * * *------------------------------------------------------------------------
 * * */
/********************************************************************
*Function main
********************************************************************/
int main(int argc, char *argv[]) {

    struct hostent *ptrh;   /* pointer to a host table entry */
    struct protoent *ptrp;  /* pointer to a protocol table entry */
    struct sockaddr_in sad; /* structure to hold an IP address */
    int sd;                 /* socket descriptor */
    int port;               /* protocol port number */
    char *host;             /* pointer to host name */
    int n;                  /* number of characters read */
    char buf[1000];         /* buffer for data from the server */
    int choice = 0;         /* For user choice*/
    char fileName[256], newFileName[256];

    memset((char *)&sad, 0, sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET;             /* set family to Internet */

    /* Check command-line argument for protocol port and extract */
    /* port number if one is specified. Otherwise, use the default */
    /* port value given by constant PROTOPORT */

    if (argc > 2) {
    /* if protocol port specified */
        port = atoi(argv[2]); /* convert to binary */
    }
    else {
        port = PROTOPORT; /* use default port number */
    }
    if (port > 0) /* test for legal value */
        sad.sin_port = htons((u_short)port);
    else
    { /* print error message and exit */
        fprintf(stderr, "bad port number %s\n", argv[2]);
        exit(1);
    }
    /* Check host argument and assign host name. */
    if (argc > 1)
    {
        host = argv[1]; /* if host argument specified */
    }
    else
    {
        host = localhost;
    }
    /* Convert host name to equivalent IP address and copy to sad. */
    ptrh = gethostbyname(host);
    if (((char *)ptrh) == NULL) {
        fprintf(stderr, "invalid host: %s\n", host);
        exit(1);
    }
    memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);
    /* Map TCP transport protocol name to protocol number. */
    if (((long)(ptrp = getprotobyname("tcp"))) == 0) {
        fprintf(stderr, "cannot map \"tcp\" to protocol number");
        exit(1);
    }
    /* Create a socket. */
    sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd < 0) {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }
    /* Connect the socket to the specified server. */
    if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr, "connect failed\n");
        exit(1);
    }

    while (1) {

        cout << endl << "Pick a choice by entering its corresponding number." << endl;

        cout << "1. DOWNLOAD: \n";
        cout << "2. UPLOAD: \n";
        cout << "3. DELETE: \n";
        cout << "4. RENAME: \n";
        cout << "5. EXIT: \n";


        cin >> choice;
        switch (choice)
        {

        case 1:
            cout << "Enter the name of the file to download: ";
            cin >> fileName;
            doDownload(fileName, sd);
            break;

        case 2:
            cout << "Enter the name of the file to upload: ";
            cin >> fileName;
            doUpload(fileName, sd);
            break;

        case 3:
            cout << "Enter the name of the file to delete: ";
            cin >> fileName;
            doDelete(fileName, sd);
            break;

        case 4:
            cout << "Enter the name of the file to rename: ";
            cin >> fileName;
            cout << "Enter the name of the new file: ";
            cin >> newFileName;
            doRename(fileName, newFileName, sd);
            break;

        case 5:
            cout << "Bye-bye! \n";
            exit(1);
        }
    }

    /* Close the socket. */
    closesocket(sd);
    /* Terminate the client program gracefully. */
    exit(0);
}

