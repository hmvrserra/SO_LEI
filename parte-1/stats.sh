#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024
##
## Aluno: Nº: 105054  Nome: Hugo Magalhães do Vale Rosa Serra
## Nome do Módulo: S4. Script: stats.sh
## Descrição/Explicação do Módulo:
##
##
###############################################################################

## S4.1. Validações:
## S4.1.1. Valida os argumentos recebidos e, conforme os mesmos, o número e tipo de argumentos recebidos. Se não respeitarem a especificação, dá so_error e termina. Caso contrário, dá so_success.

if [ "$1" == "passageiros" ] && [ -z "$2" ]; then
    so_success S4.1.1
elif [ "$1" == "top" ] && [[ "$2" =~ [0-9]+$ ]]; then
    so_success S4.1.1
else
    so_error S4.1.1
    exit 1
fi

## S4.2. Invocação do script:
## S4.2.1. Se receber o argumento passageiros, (i.e., ./stats.sh passageiros) cria um ficheiro stats.txt onde lista o nome de todos os utilizadores que fizeram reservas, por ordem decrescente de número de reservas efetuadas, e mostrando o seu valor total de compras. Em caso de erro (por exemplo, se não conseguir ler algum ficheiro necessário), dá so_error e termina. Caso contrário, dá so_success e cria o ficheiro. Em caso de empate no número de reservas, lista o primeiro do ficheiro. Preste atenção ao tratamento do singular e plural quando se escreve “reserva” no ficheiro). Um exemplo do ficheiro stats.txt será:

#TRATAR DO SINGULAR E PLURAL DE RESERVA(S)
if [ "$1" == "passageiros" ]; then
    
    > stats.txt

    nome_passageiro=$(grep ":" passageiros.txt | cut -d ':' -f3)
    #echo "$nome_passageiro"
    
    readarray -t nomes <<< "$nome_passageiro"

    for nome in "${nomes[@]}"; do

        palavra_reserva="reservas"

        user=$(grep "$nome" passageiros.txt | cut -d ':' -f1)
        nr_reservas=$(grep -o -w "$user" "relatorio_reservas.txt" | wc -l)

        valor_total=$(awk -F':' -v user="$user" '$6 == user { total += $5 } END { print total }' "relatorio_reservas.txt")
        
        if [ $nr_reservas == 0 ]; then
            valor_total=0
        fi

        if [ $nr_reservas == 1 ]; then
            palavra_reserva="reserva"
        fi

        echo "$nome: $nr_reservas $palavra_reserva; $valor_total€" >> stats.txt

    done

    if [ $? -eq 0 ]; then
        so_success S4.2.1
    else
        so_error S4.2.1
        exit 1
    fi

fi

## S4.2.2. Se receber o argumento top <nr:number>, (e.g., ./stats.sh top 4), cria um ficheiro stats.txt onde lista os <nr> (no exemplo, os 4) voos mais rentáveis (que tiveram melhores receitas de vendas), por ordem decrescente. Em caso de erro (por exemplo, se não conseguir ler algum ficheiro necessário), dá so_error e termina. Caso contrário, dá so_success e cria o ficheiro. Em caso de empate, lista o primeiro do ficheiro; o ficheiro stats.txt ficará então:

if [ "$1" == "top"  ] && [[ "$2" =~ [0-9]+$ ]]; then

    > stats.txt
    > temp_stats.txt
    > temp_stats_ordenado.txt

    nr_top="$2"
    codigo_voo=$(awk -F':' '{print $2}' relatorio_reservas.txt | sort | uniq)

    readarray -t codigos <<< "$codigo_voo"

    for codigo in "${codigos[@]}"; do

        valor_total=$(awk -F':' -v voo="$codigo" '$2 == voo { total += $5 } END { print total }' "relatorio_reservas.txt")

        echo "$codigo: $valor_total€" >> temp_stats.txt

    done

    sort -t ':' -k2,2nr temp_stats.txt > temp_stats_ordenado.txt
    
    head -n "$nr_top" temp_stats_ordenado.txt > stats.txt

    if [ $? -eq 0 ]; then
        rm temp_stats.txt
        rm temp_stats_ordenado.txt

        so_success S4.2.2
    else
        so_error S4.2.2
        exit 1
    fi

fi