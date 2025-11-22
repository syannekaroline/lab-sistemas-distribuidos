#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h> 

void error(const char *msg) // registra falhas que devem acontecer
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno; //fd - tipo inteiro
	socklen_t clilen;
	char buffer[256]; //vetor pra enviar e receber infos
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n"); //primeiro argumento -> nome do programa
		exit(1);
    }

    // cria socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // af_INET, sock_stream -> conexÃ£o tcp | se fosse datagrama ia ser udp
    if (sockfd < 0) //se retornar negativo significa que n foi criado o socket com sucesso
        error("ERROR opening socket");
        
    bzero((char *) &serv_addr, sizeof(serv_addr)); //Ã‰ uma medida de seguranÃ§a para "limpar" a variÃ¡vel serv_addr antes de usÃ¡-la, garantindo que nÃ£o haja dados antigos aleatÃ³rios nela.
    portno = atoi(argv[1]); //transforma o argumento recebido(porta) pra inteiro e atribui a variÃ¡vel na porta

    //define as variÃ¡veis da conexÃ£o- configuraÃ§Ã£o do servidor
    serv_addr.sin_family = AF_INET;//servidor usarÃ¡ a Internet (IPv4)
    serv_addr.sin_addr.s_addr = INADDR_ANY; //Aceita conexÃµes de qualquer endereÃ§o;escutar em todas as interfaces de rede que esta mÃ¡quina possui (seja Wi-Fi, Ethernet, ou o localhost 127.0.0.1
    serv_addr.sin_port = htons(portno); //"pega o nÃºmero da porta que o usuÃ¡rio digitou (o portno) e converta-o para o formato de rede ('network byte order') - Host to Network Short" (Curto - 16 bits, o tamanho de uma porta - do Host para a Rede).


    //associa o socket as configuraÃ§Ãµes definidas pro servidor
    if (bind(sockfd, (struct sockaddr *) &serv_addr, //associar o socket (sockfd) com o endereÃ§o local (IP e porta) configurados em serv_addr
        sizeof(serv_addr)) < 0) //se der -1 Ã© pq deu errado a associaÃ§Ã£o 
        error("ERROR on binding");

    
    listen(sockfd,5);//programa aguarda conexÃ£o chegar em algum momento
    clilen = sizeof(cli_addr); // Prepara uma variÃ¡vel (clilen) para dizer Ã  funÃ§Ã£o accept o tamanho da estrutura onde ela deve guardar as informaÃ§Ãµes do cliente que estÃ¡ se conectando.
    
    //PrevenÃ§Ã£o de processos zumbis
    signal(SIGCHLD, SIG_IGN);// ignora o sinal de morte do filho (SIGCHLD) -> faz com que o kernel limpe automaticamente os processos zumbis
    /*sta linha instrui o kernel a ignorar o sinal de morte do filho (SIGCHLD), o que faz com que o kernel limpe automaticamente os processos zumbis, evitando o acÃºmulo.*/

    printf("Servidor pronto, esperando conexÃµes...\n");

    while (1) {

        /* O accept() fica aguardando uma conexÃ£o atÃ© um novo cliente chegar */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,&clilen); //aceitaÃ§Ã£o de conexÃ£o

        // testa o aceite da conexÃ£o
        if (newsockfd < 0)
            error("ERROR on accept");

        /* Formk pra lidar com concorrencia de solicitaÃ§Ãµes */
        int pid = fork(); // Cria o processo filho pra atender o cliente
        if (pid < 0)
            error("ERROR on fork");

        if (pid == 0) {
            // Filho fecha o socket de escuta (nÃ£o precisa dele)
            close(sockfd); 
            printf("Cliente conectado (atendido pelo processo filho %d)\n", getpid());
        
            //Lendo a Mensagem do Cliente
            bzero(buffer,256);//limpa a memÃ³ria
            n = read(newsockfd,buffer,255);// lÃª o socket do cliente, onde armazena (buffer), o tam max pra ler "\0" finaliza uma string

            //verifica erro na leitura
            if (n < 0) 
                error("ERROR reading from socket");

            //mostra msg do cliente 
            printf("Here is the message: %s\n",buffer);
            //responde o cliente
            n = write(newsockfd,"I got your message",18);

            if (n < 0) 
                error("ERROR writing to socket");

            //fecha conexÃ£o
            close(newsockfd);
            exit(0);
        }else {
            /* fecha a conexÃ£o  do processo pai*/
            close(newsockfd);
            /* processo pai volta ao topo do 'while(1)' para esperar a prÃ³xima requisiÃ§Ã£o*/
        }
    }

    close(sockfd);
    return 0;
}