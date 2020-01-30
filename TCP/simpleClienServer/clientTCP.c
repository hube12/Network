
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

int main(int argc, char * argv[]){
    if (argc<3){
        printf("Usage is client.exe <host> <port>");
    }
    int clientSocket;
    if ((clientSocket=socket(PF_INET,SOCK_STREAM,0))<0){
        perror("error opening socket");
        exit(1);
    }
    struct sockaddr_in serv_addr;
    bzero( (char *) &serv_addr,
           sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;

    serv_addr.sin_port = htons((ushort)atoi(argv[2]));
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    if (connect (clientSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr) ) < 0){
        perror ("cliecho : erreur connect");
        exit (1);
    }
    while(1){
        char buffer[1024];
        int n;
        if ((n=recv(clientSocket, buffer, sizeof(buffer), 0) )< 0){
            perror("Error receiving");
            exit(1);
        }
        buffer[n]='\0';
        printf("%s",buffer);
        fgets(buffer,1024,stdin);
        if (send(clientSocket,buffer,strlen(buffer),0)<0){
            perror("Error sending");
            exit(1);
        }
    }
    close(clientSocket);




}