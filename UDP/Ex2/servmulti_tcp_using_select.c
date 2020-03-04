/*********************************************************************
 *                                                                   *
 * FICHIER: SERVER_TCP                                               *
 *                                                                   *
 * DESCRIPTION: Utilisation de TCP socket par une application serveur*
 *              application client                                   *
 *                                                                   *
 * principaux appels (du point de vue serveur) pour un protocole     *
 * oriente connexion:                                                *
 *     socket()                                                      *
 *                                                                   * 
 *     bind()                                                        *
 *                                                                   * 
 *     listen()                                                      *
 *                                                                   *
 *     accept()                                                      *
 *                                                                   *
 *     read()                                                        *
 *                                                                   *
 *     write()                                                       *
 *                                                                   *
 *     select()                                                      *
 *********************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 1500

int str_echo(int sockfd) {
    int nrcv, nsnd;
    char msg[BUFSIZE];

    /*    * Attendre  le message envoye par le client
     */
    memset((char *) msg, 0, sizeof(msg));
    if ((nrcv = read(sockfd, msg, sizeof(msg) - 1)) < 0) {
        perror("servmulti : : readn error on socket");
        exit(1);
    }
    msg[nrcv] = '\0';
    printf("servmulti :message recu=%s du processus %d nrcv = %d \n", msg, getpid(), nrcv);

    if ((nsnd = write(sockfd, msg, nrcv)) < 0) {
        printf("servmulti : writen error on socket");
        exit(1);
    }
    printf("nsnd = %d \n", nsnd);
    return (nsnd);
} /* end of function */


usage() {
    printf("usage : servmulti numero_port_serveur\n");
}


int main(int argc, char *argv[]) {
    int sockfd, n, newsockfd, childpid, servlen, fin;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    /* Verifier le nombre de paramètre en entrée */
    if (argc != 2) {
        usage();
        exit(1);
    }


/*
 * Ouvrir une socket (a TCP socket)
 */
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("servmulti : Probleme socket\n");
        exit(2);
    }


/*
 * Lier l'adresse  locale à la socket
 */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));


    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("servmulti : erreur bind\n");
        exit(1);
    }

/* Paramètrer le nombre de connexion "pending" */
    if (listen(sockfd, SOMAXCONN) < 0) {
        perror("servmulti : erreur listen\n");
        exit(1);
    }
    int tab_clients[FD_SETSIZE];
    fd_set current_set, old_set;
    FD_ZERO(&current_set);
    FD_ZERO(&old_set);

    FD_SET(sockfd, &old_set);
    int maxfdp = sockfd + 1;
    for (int i = 0; i < FD_SETSIZE; ++i) {
        tab_clients[i] = -1;
    }
    int dialog_socket;
    for (;;) {

        current_set = old_set;
        int nbfd = select(maxfdp, &current_set, NULL, NULL, NULL);
        if (FD_ISSET(sockfd, &current_set)) {
            dialog_socket = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
            if (dialog_socket < 0) {
                perror("ERROR while opening dialog socket");
                exit(1);
            }
            int i = 0;
            //find first available place in the tab
            while ((i < FD_SETSIZE) && tab_clients[i] >= 0) {
                i++;
            }
            if (i == FD_SETSIZE) {
                perror("ERROR, no space left in tab, too many clients");
                exit(1);
            }
            tab_clients[i] = dialog_socket;
            printf("client #%d was added\n",dialog_socket);
            FD_SET(dialog_socket, &old_set);
            if (dialog_socket >= maxfdp) {
                maxfdp = dialog_socket + 1;
            }
            nbfd--;

        }
        int i = 0;
        while ((nbfd > 0) && (i < FD_SETSIZE)) {
            if (tab_clients[i] >= 0) {
                int sockcli = tab_clients[i];
                //if client is find in select
                if (FD_ISSET(sockcli, &current_set)) {
                    printf("client #%d is handled\n",sockcli);
                    // if client want to close connection
                    if (str_echo(sockcli) == 0) {
                        close(sockcli);
                        tab_clients[i] = -1;
                        FD_CLR(sockcli, &old_set);
                        printf("client #%d was removed\n",sockcli);
                    }
                    nbfd--;
                }
            }
            i++;
        }
    }
}



















