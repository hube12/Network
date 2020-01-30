#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cygwin/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define SERV_PORT 5003

int main() {
    struct sockaddr_in serv_addr;
    int serverSocket;
    int true = 1;
    if ((serverSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("error opening socket");
        exit(1);
    }
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);
    if (bind(serverSocket, (struct sockaddr *) &serv_addr, sizeof
    (serv_addr)) < 0) {
        perror("servecho: erreur bind\n");
        exit(1);
    }

    if (listen(serverSocket,
               SOMAXCONN) < 0) {
        perror("servecho: erreur listen\n");
        exit(1);
    }
    int dialogSocket;
    int clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);
    dialogSocket = accept(serverSocket,
                          (struct sockaddr *) &cli_addr,
                          (socklen_t *) &clilen);
    if (dialogSocket < 0) {
        printf("servecho : erreur accept\n");
        exit(1);
    }
        switch (fork()) {
            case -1 :
                printf("Fork failed\n");
                exit(1);
            case 0:
                close(serverSocket);
                const char *msg = "bonjour\n";
                if (send(dialogSocket, msg, strlen(msg), 0) < 0) {
                    printf("Error sending\n");
                    exit(1);
                }
                while(1) {
                    char buffer[1024];
                    int n;
                    if ((n=recv(dialogSocket, buffer, sizeof(buffer), 0) )< 0) {
                        printf("Error receiving\n");
                        exit(1);
                    }
                    buffer[n]='\0';
                    printf("%s", buffer);
                    fgets(buffer, 1024, stdin);
                    if (send(dialogSocket, buffer, strlen(buffer), 0) < 0) {
                        printf("Error sending\n");
                        exit(1);
                    }
                }
                close(dialogSocket);
                exit(0);
            default:
                wait(NULL);
                close(dialogSocket);
        }



    close(serverSocket);
}
