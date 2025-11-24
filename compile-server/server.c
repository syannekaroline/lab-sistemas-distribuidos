#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void dossstuff(int sock) {
    int n;
    char buffer[4096];
    char file_buffer[1024]; // Buffer auxiliar para leitura de arquivo binário
    char filename[32], exe_name[32], out_name[32], cmd_compile[128], cmd_run[128];
    FILE *fp;

    // Identificador único (PID)
    int pid = getpid();
    sprintf(filename, "temp_%d.c", pid);
    sprintf(exe_name, "./temp_%d", pid); // Nome do executável
    sprintf(out_name, "out_%d.txt", pid);

    // 1. Ler dados do socket
    bzero(buffer, 4096);
    n = read(sock, buffer, 4095);
    if (n < 0) error("ERROR reading from socket");

    // 2. Verificar o Modo (Protocolo simples)
    // Se começar com "MODE:DL", o cliente quer baixar o binário.
    int mode_download = 0;
    char *code_start = buffer;

    if (strncmp(buffer, "MODE:DL", 7) == 0) {
        mode_download = 1;
        code_start = buffer + 8; // Pula o cabeçalho "MODE:DL\n" (aprox)
    } else if (strncmp(buffer, "MODE:RUN", 8) == 0) {
        mode_download = 0;
        code_start = buffer + 9; // Pula o cabeçalho "MODE:RUN\n"
    }

    // 3. Salvar o código recebido (ignorando o cabeçalho)
    fp = fopen(filename, "w");
    if (fp == NULL) error("ERROR creating file");
    fprintf(fp, "%s", code_start);
    fclose(fp);

    // 4. Compilar
    // Redireciona erro de compilação para o arquivo de log
    sprintf(cmd_compile, "gcc %s -o %s 2> %s", filename, exe_name, out_name);
    int compile_status = system(cmd_compile);

    if (compile_status != 0) {
        // Se falhou, envia o erro (mesmo se pediu download, não tem o que baixar)
        fp = fopen(out_name, "r");
        write(sock, "[ERRO DE COMPILACAO]:\n", 22);
        while (fgets(buffer, 4095, fp) != NULL) {
            write(sock, buffer, strlen(buffer));
        }
        fclose(fp);
    }
    else {
        // COMPILAÇÃO SUCESSO
        if (mode_download == 1) {
            // --- MODO DOWNLOAD: Enviar o arquivo binário ---
            fp = fopen(exe_name, "rb"); // Importante: rb (leitura binária)
            if (fp != NULL) {
                while ((n = fread(file_buffer, 1, sizeof(file_buffer), fp)) > 0) {
                    write(sock, file_buffer, n);
                }
                fclose(fp);
            }
        } else {
            // Executar e enviar texto
            sprintf(cmd_run, "%s >> %s 2>&1", exe_name, out_name);
            system(cmd_run);

            fp = fopen(out_name, "r");
            if (fp != NULL) {
                while (fgets(buffer, 4095, fp) != NULL) {
                    write(sock, buffer, strlen(buffer));
                }
                fclose(fp);
            }
        }
    }

    // 5. Limpeza
    remove(filename);
    remove(exe_name);
    remove(out_name);

    close(sock);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");
        int pid = fork();
        if (pid == 0) {
            close(sockfd);
            dossstuff(newsockfd);
            exit(0);
        } else {
            close(newsockfd);
            waitpid(-1, NULL, WNOHANG);
        }
    }
    return 0;
}
