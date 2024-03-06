#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024
##
## Aluno: Nº: 105054 Nome: Hugo Magalhães do Vale Rosa Serra
## Nome do Módulo: S5. Script: menu.sh
## Descrição/Explicação do Módulo:
##
##
###############################################################################

## S5.1. Apresentação:
while true; do
    ## S5.1.1. O script apresenta (pode usar echo, cat ou outro, sem “limpar” o ecrã) um menu com as opções abaixo indicadas.
    
    echo " "
    echo "MENU:"
    echo "1: Regista/Atualiza saldo do passageiro"
    echo "2: Reserva/Compra de bilhetes"
    echo "3: Atualiza Estado dos voos"
    echo "4: Estatísticas - Passageiros"
    echo "5: Estatísticas - Top Voos + Rentáveis"
    echo "0: Sair"
    echo " "

    read -p "Opção: " opcao
    echo " "

    ## S5.2. Validações:
    ## S5.2.1. Aceita como input do utilizador um número. Valida que a opção introduzida corresponde a uma opção válida. Se não for, dá so_error <opção> (com a opção errada escolhida), e volta ao passo S5.1 (ou seja, mostra novamente o menu). Caso contrário, dá so_success <opção>.

    if [[ $opcao -lt 0 ]] || [[ $opcao -gt 5 ]]; then
        so_error S5.2.1 "$opcao"
    else
        so_success S5.2.1 $opcao
    fi

    if [[ $opcao == 0 ]]; then
        exit 1
    fi

    ## S5.2.2. Analisa a opção escolhida, e mediante cada uma delas, deverá invocar o sub-script correspondente descrito nos pontos S1 a S4 acima. No caso das opções 1 e 4, este script deverá pedir interactivamente ao utilizador as informações necessárias para execução do sub-script correspondente, injetando as mesmas como argumentos desse sub-script:
    ## S5.2.2.1. Assim sendo, no caso da opção 1, o script deverá pedir ao utilizador sucessivamente e interactivamente os dados a inserir:

    if [[ $opcao == 1 ]]; then
        echo "Regista passageiro / Atualiza saldo passageiro:"
        read -p "Indique o nome: " nome
        read -p "Indique a senha: " senha
        read -p "Para registar o passageiro, insira o NIF: " nif
        read -p "Indique o saldo a adicionar ao passageiro: " saldo_adicionar

        if [ -z "$nif" ]; then
            ./regista_passageiro.sh "$nome" "$senha" "$saldo_adicionar"
        else
            ./regista_passageiro.sh "$nome" "$senha" "$saldo_adicionar" "$nif"
        fi
        
        so_success S5.2.2.1
    fi

    ## S5.2.2.2. No caso da opção 2, o script invoca o Sub-Script: compra_bilhete.sh. Após a execução do sub-script, dá so_success e volta para o passo S5.1.

    if [[ $opcao == 2 ]]; then
        ./compra_bilhete.sh
        so_success S5.2.2.2
    fi

    ## S5.2.2.3. No caso da opção 3, o script invoca o Sub-Script: estado_voos.sh. Após a execução do sub-script, dá so_success e volta para o passo S5.1.

    if [[ $opcao == 3 ]]; then
        ./estado_voos.sh
        so_success S5.2.2.3
    fi

    ## S5.2.2.4. No caso da opção 4, o script invoca o Sub-Script: stats.sh com o argumento necessário. Após a execução do sub-script, dá so_success e volta para o passo S5.1.

    if [[ $opcao == 4 ]]; then
        ./stats.sh passageiros
        so_success S5.2.2.4
    fi

    ##S5.2.2.5. No caso da opção 5, o script deverá pedir ao utilizador o nº de voos a listar, antes de invocar o Sub-Script: stats.sh:

    if [[ $opcao == 5 ]]; then
        read -p "Indique o número de voos a listar: " nr_voos
        ./stats.sh top $nr_voos
        so_success S5.2.2.5
    fi
done