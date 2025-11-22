#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
void error(const char *msg)
{
    perror(msg);
    exit(0);
}
int main(int argc, char *argv[])
{   
    //ConfiguraÃ§Ã£o Inicial e VerificaÃ§Ã£o de Argumentos
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;struct hostent *server;
    char buffer[256];
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);//converte porta pra inteiro e armazena
    sockfd = socket(AF_INET, SOCK_STREAM, 0);//cria o socket
    //testa criaÃ§Ã£o
    if (sockfd < 0)
        error("ERROR opening socket");


    server = gethostbyname(argv[1]);//pega o ip real do host
    //testa o ip
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }


    bzero((char *) &serv_addr, sizeof(serv_addr)); //zera/limpa memÃ³ria
    serv_addr.sin_family = AF_INET; // define conf ipv4

    memcpy(&serv_addr.sin_addr.s_addr, (char *)server->h_addr_list[0], server->h_length);

    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    
    char *message = "Teste de estresse";
    bzero(buffer,256);
    strcpy(buffer, message);

    n = write(sockfd,buffer,strlen(buffer));

    if (n < 0)
        error("ERROR writing to socket");

    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0)
        error("ERROR reading from socket");

    printf("%s\n",buffer);
    close(sockfd);

    return 0;
}