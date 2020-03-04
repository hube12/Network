


/*********************************************************************
 *                                                                   *
 * FICHIER: SERVER_UDP                                               *
 *                                                                   *
 * DESCRIPTION: Utilisation de UDP socket par une application server *
 *                                                                   *
 * principaux appels (du point de vue client)                        *
 *     socket()                                                      *
 *                                                                   *
 *     sendto()                                                      *
 *                                                                   *
 *     recvfrom()                                                    *
 *                                                                   *
 *********************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 80
#define BUFSIZE 1024

int main(int argc, char *argv[]) {

    int sockfd;        /* socket */
    int portno;        /* port to listen on */
    unsigned int clientlen;        /* byte size of client's address */
    struct sockaddr_in serveraddr;    /* server's addr */
    struct sockaddr_in clientaddr;    /* client addr */
    struct hostent *hostp;    /* client host info */
    char *buf;        /* message buf */
    char *hostaddrp;    /* dotted decimal host addr string */
    int optval;        /* flag value for setsockopt */
    int n;            /* message byte size */



    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    char *endptr;
    portno = (int)strtol(argv[1], &endptr,10);

    //create Socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    /*
     * Remplir la structure  serv_addr avec l'adresse du serveur
     */
    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
               (const void *) &optval, sizeof(int));

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short) portno);

    if (bind(sockfd, (struct sockaddr *) &serveraddr,
             sizeof(serveraddr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }


    clientlen = sizeof(clientaddr);
    while (1) {

        /*
         * recvfrom: receive a UDP datagram from a client
         */
        buf = malloc(BUFSIZE);
        n = recvfrom(sockfd, buf, BUFSIZE, 0,
                     (struct sockaddr *) &clientaddr, &clientlen);
        if (n < 0) {
            perror("ERROR in recvfrom");
            exit(1);
        }

        /*
         * gethostbyaddr: determine who sent the datagram
         */
        hostp = gethostbyaddr((const char *) &clientaddr.sin_addr.s_addr,
                              sizeof(clientaddr.sin_addr.s_addr),
                              AF_INET);
        if (hostp == NULL) {

            perror("ERROR on gethostbyaddr");
            exit(1);
        }
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL) {

            perror("ERROR on inet_ntoa");
            exit(1);
        }
        printf("server received %d bytes containing message : %s from %s \n", n,buf,hostaddrp);

        /*
         * sendto: echo the input back to the client
         */
        n = sendto(sockfd, buf, n, 0,
                   (struct sockaddr *) &clientaddr, clientlen);
        if (n < 0) {

            perror("sendto");
            exit(1);
        }
    }
    close(sockfd);
}
