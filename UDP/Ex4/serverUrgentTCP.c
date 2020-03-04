/*********************************************************************
 *                                                                   *
 * FICHIER: SERVER_URGENT_TCP                                        *
 *                                                                   *
 * DESCRIPTION: Utilisation de la donnee urgente de TCP               *
 *                                                                   *
 *********************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <zconf.h>

int serverSocket, clientSocket; /* declaration socket passive et socket active */

int str_echo (int sockfd)
{
    int nrcv, nsnd;
    char msg[2048];

    /*    * Attendre  le message envoye par le client
     */
    memset( (char*) msg, 0, sizeof(msg) );
    if ( (nrcv= read ( sockfd, msg, sizeof(msg)-1) ) < 0 )  {
        perror ("servmulti : : readn error on socket");
        exit (1);
    }
    msg[nrcv]='\0';
    printf ("servmulti :message recu=%s du processus %d nrcv = %d \n",msg,getpid(), nrcv);

    if ( (nsnd = write (sockfd, msg, nrcv) ) <0 ) {
        printf ("servmulti : writen error on socket");
        exit (1);
    }
    printf ("nsnd = %d \n", nsnd);
    return (nsnd);
} /* end of function */
void sig_urg(int sig)
{
    int n=1;
    char buff[2048];
    printf ("SIGURG recu \n");


    printf("Recepteur: recu signal SIGURG (%d): caractere urgent\n",sig);
    /* Reception du caractere urgent */


    printf ("read %d OOB octets %s\n", n,buff);
}

usage(){
    printf("usage : servecho numero_port_serveur\n");
}


int main (int argc, char *argv[])

{
    struct sigaction action;

    int n, clilen, servlen, size;
    struct sockaddr_in  serv_addr, cli_addr;
    char buff[2048];
    socklen_t optionlen;

    /* Verifier le nombre de paramètre en entrée */
    /* serverTCP <numero_port>                   */
    if (argc != 2){
        usage();
        exit(1);
    }

    // Ouvrir une socket (a TCP socket)
    if ((serverSocket = socket(PF_INET, SOCK_STREAM, 0)) <0) {
        perror("servecho : Probleme socket\n");
        exit (2);
    }

    // Lier l'adresse  locale à la socket
    memset( (char*) &serv_addr, 0, sizeof(serv_addr) );
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serverSocket,(struct sockaddr *)&serv_addr, sizeof(serv_addr) ) <0) {
        perror ("servecho: erreur bind\n");
        exit (1);
    }
    size= sizeof(buff);
    //size=getsockopt(serverSocket,SOL_SOCKET,SO_RCVBUF,NULL,NULL);

    printf (" la taille du buffer de reception est de %d \n", size);



    // Paramètrer le nombre de connexion "pending"
    if (listen(serverSocket, SOMAXCONN) <0) {
        perror ("servecho: erreur listen\n");
        exit (1);
    }

    // Attente de la demande de connexion du client
    clilen = sizeof(cli_addr);
    clientSocket = accept(serverSocket, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
    if (clientSocket < 0) {
        perror("servecho : erreur accept\n");
        exit (1);
    }


    sigemptyset(&action.sa_mask);         /* clear the signal set */
    action.sa_flags = 0;                  /* enables setting sa_handler, not sa_action */
    action.sa_handler = sig_urg;         /* specify a handler */
    sigaction(SIGURG, &action, NULL);



    // Demander au systeme d'envoyer le signal au proprietaire de la socket
    // A COMPLETER
    kill(clientSocket,SIGURG);


    for (;;)
    {
        // Reception des donnees non urgents. Si Fin de connexion ou erreur
        // sortir du serveur
        // Attention au traitement de l'erreur EINTR
        // A COMPLETER
        if (str_echo (clientSocket)==0){
            close(serverSocket);
            exit(-1);
        }


    }


}
