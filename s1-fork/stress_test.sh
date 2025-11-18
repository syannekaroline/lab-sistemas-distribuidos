#!/bin/bash

# configurações de ip e porta
HOST="127.0.0.1"
PORT="51482" 

# numero de clientes do teste
NUM_CLIENTS=100000 

echo "Iniciando teste de estresse com $NUM_CLIENTS clientes..."

# teste de estresse
for i in $(seq 1 $NUM_CLIENTS)
do
   # & -> terminal roda arquivo cliente em "background" e pula para o próximo, sem esperar o anterior terminar.
   ./stress_client $HOST $PORT &
done

echo "Todos os $NUM_CLIENTS clientes foram lançados."
echo "Aguardando todos terminarem..."
wait # Espera todos os processos em background terminarem
echo "Teste concluído."
