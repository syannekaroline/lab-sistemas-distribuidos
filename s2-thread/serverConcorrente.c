#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>

#define MAX_QUEUE 100000       // Capacidade da fila de conexões
#define THREAD_POOL_SIZE 8    // Número de threads no pool
#define BUFFER_SIZE 256

// ------------------
// FILA DE CONEXÕES
// ------------------

int connection_queue[MAX_QUEUE];
int queue_start = 0;
int queue_end = 0;
int queue_count = 0;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

void queue_push(int client_sock) {
    pthread_mutex_lock(&queue_mutex);

    if (queue_count < MAX_QUEUE) {
        connection_queue[queue_end] = client_sock;
        queue_end = (queue_end + 1) % MAX_QUEUE; // [NULL, NULL, 3, ...,9999] -> 9999 + 1 = 10000 % 10000 = 0 (circular)
        queue_count++;
        pthread_cond_signal(&queue_not_empty); // acorda um worker
    } else {
        // Fila cheia → rejeita cliente
        close(client_sock);
    }

    pthread_mutex_unlock(&queue_mutex);
}

int queue_pop() {
    pthread_mutex_lock(&queue_mutex);

    while (queue_count == 0) {
        pthread_cond_wait(&queue_not_empty, &queue_mutex); // coloca a thread para dormir
    }

    int client_sock = connection_queue[queue_start];
    queue_start = (queue_start + 1) % MAX_QUEUE; // [NULL, NULL, 3, ...,9999] -> 9999 + 1 = 10000 % 10000 = 0 (circular)
    queue_count--;

    pthread_mutex_unlock(&queue_mutex);
    return client_sock;
}

// ------------------
// THREAD WORKER
// ------------------

void* worker_thread(void *arg) {
    char buffer[BUFFER_SIZE];

    while (1) {
        int client_sock = queue_pop();

        bzero(buffer, BUFFER_SIZE);
        int n = read(client_sock, buffer, BUFFER_SIZE - 1);

        if (n > 0) {
            printf("[Thread %ld] Recebido: %s\n", pthread_self(), buffer);

            write(client_sock, "I got your message", 18);
        }

        close(client_sock);
    }
    return NULL;
}

// ------------------
// SERVIDOR PRINCIPAL
// ------------------

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr,"ERRO: informe a porta\n");
        exit(1);
    }


    // Criar socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, SOMAXCONN);  // máximo possível

    clilen = sizeof(cli_addr);

    printf("Servidor com pool de threads iniciado na porta %d\n", portno);

    // Criar o pool de threads
    pthread_t threads[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

    // Loop principal: apenas aceitar e enfileirar
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) {
            perror("ERROR on accept");
            continue;
        }

        // Enfileira para o pool processar
        queue_push(newsockfd);
    }

    close(sockfd);
    return 0;
}
