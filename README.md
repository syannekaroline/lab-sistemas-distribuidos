# Trabalho de labratório de Sistemas distribuídos: Sockets em Linux

Este repositório contém a resolução do trabalho da disciplina de Sistemas distribuídos, focado na implementação de um servidor de sockets em C capaz de lidar com múltiplas conexões e executar um serviço de compilação remota.


## Descrição do Trabalho

* O objetivo deste trabalho é evoluir um servidor de socket TCP básico, que inicialmente só atende um cliente por vez, para uma aplicação robusta capaz de gerenciar múltiplas requisições de clientes de forma concorrente.

* O projeto final consiste em um **servidor de compilação** e um **cliente com interface gráfica (GUI)** para interagir com este serviço.

---

## 🚀 Questões Propostas

Abaixo estão as questões do trabalho e as seções para as respostas e implementações correspondentes.

### Questão 1: Soluções para Múltiplos Clientes

> Quais as soluções possíveis para que o programa servidor possa atender a uma quantidade indeterminada de requisições de programas clientes? 

#### Resposta

Conforme [M. 2008 pp. 36, 68, 121–122], uma das mais simples técnicas para implementar um servidor concorrente é por meio de funções fork do unix, que é capaz de criar processo-filho para atender múltiplos clientes ao mesmo tempo. Para isso, o servidor aceita conexões clientes e bifurca a requisição com a chamada da função fork para criar uma cópia de si próprio (Figura 2.13), deixando o processo-filho atender a requisição do cliente. Dessa forma o processo-pai espera novas conexões e cria novos processos-filho a cada nova requisição(Figura 2.14). Assim, é capaz de atender a uma quantidade indeterminada de requisições de programas clientes.

 ![texto alternativo](https://private-user-images.githubusercontent.com/87232098/515417264-39d48879-2b87-4fff-b5da-03b5193830d5.png?jwt=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3NjM0MTg2MzcsIm5iZiI6MTc2MzQxODMzNywicGF0aCI6Ii84NzIzMjA5OC81MTU0MTcyNjQtMzlkNDg4NzktMmI4Ny00ZmZmLWI1ZGEtMDNiNTE5MzgzMGQ1LnBuZz9YLUFtei1BbGdvcml0aG09QVdTNC1ITUFDLVNIQTI1NiZYLUFtei1DcmVkZW50aWFsPUFLSUFWQ09EWUxTQTUzUFFLNFpBJTJGMjAyNTExMTclMkZ1cy1lYXN0LTElMkZzMyUyRmF3czRfcmVxdWVzdCZYLUFtei1EYXRlPTIwMjUxMTE3VDIyMjUzN1omWC1BbXotRXhwaXJlcz0zMDAmWC1BbXotU2lnbmF0dXJlPWZiODAwODBkNmM5ZTgyNzIxMTIyZTE3NjUyZmZiYzM4MWFjZDkxOWY4YTM3N2I0NmE4MzE4ZjUzZmU0ZWI3NDAmWC1BbXotU2lnbmVkSGVhZGVycz1ob3N0In0.r0Ellhvq1-brHLy29QGZf_PqqdbaR0fub6NPh5IFpYQ)
 
![texto alternativo](https://private-user-images.githubusercontent.com/87232098/515417677-0322bf10-06d4-4134-a8d7-57a34487c7f9.png?jwt=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3NjM0MTg1NDYsIm5iZiI6MTc2MzQxODI0NiwicGF0aCI6Ii84NzIzMjA5OC81MTU0MTc2NzctMDMyMmJmMTAtMDZkNC00MTM0LWE4ZDctNTdhMzQ0ODdjN2Y5LnBuZz9YLUFtei1BbGdvcml0aG09QVdTNC1ITUFDLVNIQTI1NiZYLUFtei1DcmVkZW50aWFsPUFLSUFWQ09EWUxTQTUzUFFLNFpBJTJGMjAyNTExMTclMkZ1cy1lYXN0LTElMkZzMyUyRmF3czRfcmVxdWVzdCZYLUFtei1EYXRlPTIwMjUxMTE3VDIyMjQwNlomWC1BbXotRXhwaXJlcz0zMDAmWC1BbXotU2lnbmF0dXJlPWIzMjI0YjQxNDk4MzFlODYwMWIyYWNhYTllZjIyMzI3NTVlNzUxOThlYmZlNmE3ZmE5Y2ZmYzY4ZjVjNzRmOGEmWC1BbXotU2lnbmVkSGVhZGVycz1ob3N0In0.ea3Y0Arp53pM7I7wNYSTNnMCIXzSGkjp60xOK8Iki18)

Na prática, conforme a imagem abaixo, o processo servidor (Pai) opera em um loop infinito, aguardando conexões bloqueado na chamada accept. Quando uma conexão é estabelecida, o servidor cria um processo-filho via fork(). O processo Pai imediatamente fecha sua cópia do socket conectado e volta a esperar por novas conexões em accept. O processo Filho, por sua vez, fecha o socket de escuta (pois não o usará), processa toda a comunicação com o cliente através do socket conectado e, ao finalizar, fecha o socket conectado e termina sua execução.

![texto alternativo](https://private-user-images.githubusercontent.com/87232098/515419686-9dbc58f1-d43c-4274-bdbc-ef5d991a4aee.png?jwt=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3NjM0MTg4MDEsIm5iZiI6MTc2MzQxODUwMSwicGF0aCI6Ii84NzIzMjA5OC81MTU0MTk2ODYtOWRiYzU4ZjEtZDQzYy00Mjc0LWJkYmMtZWY1ZDk5MWE0YWVlLnBuZz9YLUFtei1BbGdvcml0aG09QVdTNC1ITUFDLVNIQTI1NiZYLUFtei1DcmVkZW50aWFsPUFLSUFWQ09EWUxTQTUzUFFLNFpBJTJGMjAyNTExMTclMkZ1cy1lYXN0LTElMkZzMyUyRmF3czRfcmVxdWVzdCZYLUFtei1EYXRlPTIwMjUxMTE3VDIyMjgyMVomWC1BbXotRXhwaXJlcz0zMDAmWC1BbXotU2lnbmF0dXJlPTIyZTg4ZmJhZjg0MGFmY2U5ZDE1YzhkNmI4OGQ4OWJiMjIyMWRiOTY2MTY5ZmNlNWY2ZjljZmVlNzg5YzI4YjkmWC1BbXotU2lnbmVkSGVhZGVycz1ob3N0In0.y3bGB0OmQpnJnUkMNmZeKsDnC0C4zoS4BQVTljoLLOg)

**Referências**

M., S., W. Richard; Fenner, Bill; Rudoff, Andrew (2008). Programação de Rede Unix - Api para Soquetes de Rede. 

---

### Questão 2: Implementação e Teste de Stress

> Implemente todas as soluções determinadas e execute um teste de stress para verificar até quantas requisições o servidor/sistema operacional pode atender sem apresentar problemas.

#### Implementações

* **Solução 1 - Fork:** `[ /s1-fork/]`

#### Resultados do Teste de Stress

**Solução 1 - fork**

O teste foi realizado em uma máquina Dell Inspiron 3501 com 12,0 GiB de RAM e Ubuntu 22.04.4 LTS . A metodologia consistiu em iniciar o servidor e, em outro terminal, executar um script stress_test.sh. Este script foi configurado para o host local (127.0.0.1) e porta 51482 , lançando um número variável de clientes (NUM_CLIENTS) em background (&) e aguardando todos terminarem com o comando wait. Os testes foram executados em etapas, aumentando o número de clientes de 100 até 100.000. Os resultados mostraram que, embora tenham ocorrido erros de "Connection reset by peer" nos testes de 1.000 e 2.000 clientes, os testes com 50.000 e 100.000 clientes foram concluídos com sucesso. O teste de 100.000 clientes foi o último e mais longo da bateria, levando 6 minutos e 37,757 segundos para terminar, sendo esse o limite máximo de clientes avaliado neste experimento, mesmo sem apresentar erros.

| Número de clientes | Erros observados (cliente) | Erros observados (servidor) | Duração Total (real) |
| :--- | :--- | :--- | :--- |
| 100 |  | | 0m1,069s |
| 500 |  | | 0m1,170s |
| 1000 | 240 vzs - ERROR reading from socket: Connection reset by peer |  | 2m0,428s |
| 2000 | 88 vzs - ERROR reading from socket: Connection reset by peer | | 2m2,848s |
| 4000 |  | | 0m3,000s |
| 8000 |  | | 0m4,558s |
| 12000 |  | | 0m8,331s |
| 50000 |  | | 2m4,123s |
| 100000 |  | | 6m37,757s |

---

### Questão 3: Servidor de Compilação e Cliente GUI

> Com a solução determinada no passo anterior, crie um servidor que efetue os serviços abaixo: 

#### a. Servidor de Compilação

O servidor implementado utiliza a abordagem de `[Solução escolhida na Questão 2]` e oferece suporte para a compilação e execução da linguagem **[Linguagem Escolhida, ex: C, Python, etc.]**.

* **Código-fonte:** `[Link para a pasta/código, ex: /q3/compilation_server/]`
* **Protocolo de Comunicação:** `[Descreva brevemente como o cliente e o servidor se comunicam. Ex: O cliente envia o código-fonte, o servidor retorna duas mensagens: 1. Saída de Erros, 2. Saída Padrão.]`

#### b.Cliente com Interface Gráfica (GUI) 

Foi desenvolvido um cliente local para Linux com interface gráfica que permite ao usuário interagir com o servidor de compilação.

* **Código-fonte:** `[Link para a pasta/código, ex: /q3/gui_client/]`
* **Tecnologia/Biblioteca Gráfica:** `[Ex: GTK, Qt, Tkinter, etc.]`
* **Funcionalidades Implementadas:**
    * [x] Área para edição de programa.
    * [x] Área para retornar os erros de compilação.
    * [x] Área para retorno da saída do programa.
    * [x] Botão para execução (enviar para o servidor).
    * [ ] Botão para baixar o programa compilado (desejável).

#### Screenshot do Cliente

`[Insira aqui uma imagem do seu cliente GUI em funcionamento, similar à Figura 01 do PDF.]`
`![Screenshot do Cliente GUI](caminho/para/imagem.png)`

---

## 🛠️ Como Compilar e Executar

### Pré-requisitos

`[Liste as dependências necessárias. Ex:
* gcc
* make
* Bibliotecas de desenvolvimento do GTK (libgtk-3-dev)
* Python 3.x (para o servidor de compilação)]`

### 1. Servidor de Compilação (Questão 3a)

```bash
# Navegue até o diretório do servidor
cd q3/compilation_server/

# Compile o servidor (ajuste conforme necessário)
gcc server_q3.c -o server_q3 -lpthread

# Execute o servidor informando a porta
./server_q3 9999
