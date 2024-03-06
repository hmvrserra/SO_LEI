#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024
##
## Aluno: Nº: 105054   Nome: Hugo Magalhães do Vale Rosa Serra
## Nome do Módulo: S2. Script: compra_bilhete.sh
## Descrição/Explicação do Módulo:
##
##
###############################################################################

## S2.1. Validações e Pedido de informações interativo:
## S2.1.1. O script valida se os ficheiros voos.txt, passageiros.txt e estado_voos.txt existem. Se algum não existir, dá so_error e termina. Caso contrário, dá so_success.

if [ -e voos.txt ] && [ -e passageiros.txt ]; then
    so_success S2.1.1
else
    so_error S2.1.1
    exit 1
fi

## S2.1.2. Na plataforma é possível consultar os voos pela sua <Origem> ou <Destino>. Peça:"Insira a cidade de origem ou destino do voo:". O utilizador insere a cidade Origem ou Destino do voo (o interesse é que pesquise nos 2 campos). Caso o utilizador tenha introduzido uma cidade que não exista no ficheiro voos.txt, ou se não existirem voos disponíveis com origem ou destino nessa cidade, no ficheiro estado_voos.txt, dá so_error e termina; Caso contrário, dá so_success <Cidade>. A disponibilidade de um Voo é verificada pela sua entrada no ficheiro estado_voos.txt: há disponibilidade de lugares se o campo <Status> for “Available”.

read -p "Insira a cidade de origem ou destino do voo: " cidade

#Acusa erro caso o input por parte do user tenha sido vazio
if [[ -z $cidade ]]; then
    so_error S2.1.2
    exit 1
fi

# Verifica se a cidade existe em voos.txt
if grep -q -w "$cidade" voos.txt; then

    # Vê os lugares disponiveis por cada linha com o nome da cidade inserida
    codigo_voo=$(grep "$cidade" voos.txt | cut -d ':' -f1)
    
    # Transforma o temp em um array de nome codigos que guarda todos os codigos de voos com a cidade inserida
    readarray -t codigos <<< "$codigo_voo"
    index=0
    length=${#codigos[@]}
    
    #Boolean validador
    valido=false

    #Array que ira guardar apenas os odigos de voo com lugares disponiveis acima de 0
    codigos_validos=()

    # Verifica se ha voos disponiveis
    while [ "$index" -lt "$length" ]; do
        disponibilidade=$(grep "${codigos[$index]}" voos.txt | cut -d ':' -f8)
        if [ "$disponibilidade" -gt 0 ]; then
            codigos_validos+=("${codigos[$index]}") #Adiciona ao array um voo com disponibilidade superior a 0
            valido=true 
        fi
        ((index++))
    done

    #Verifica se encontrou pelo menos um voo disponivel
    if [ $valido == false ]; then 
        so_error S2.1.2
        exit 1
    else
        so_success S2.1.2 "$cidade"
    fi

else
    so_error S2.1.2
    exit 1
fi


## S2.1.3. O programa pede ao utilizador para inserir uma opção de voo, listando os voos que existem de acordo com a origem/destino inserida anteriormente, da seguinte forma: O utilizador insere a opção do voo (neste exemplo, números de 1 a 3 ou 0). Se o utilizador escolheu um número de entre as opções de voos apresentadas (neste caso, entre 1 e 3), dá so_success <opção>. Caso contrário, dá so_error e termina.

index2=0
length2=${#codigos_validos[@]}

 while [ "$index2" -lt "$length2" ]; do
    opcao=$((1 + index2))
    origem=$(grep "${codigos_validos[$index2]}" voos.txt | cut -d ':' -f2)
    destino=$(grep "${codigos_validos[$index2]}" voos.txt | cut -d ':' -f3)
    data_partida=$(grep "${codigos_validos[$index2]}" voos.txt | cut -d ':' -f4)
    hora_partida=$(grep "${codigos_validos[$index2]}" voos.txt | cut -d ':' -f5)
    preco=$(grep "${codigos_validos[$index2]}" voos.txt | cut -d ':' -f6)
    lugares_disponiveis=$(grep "${codigos_validos[$index2]}" voos.txt | cut -d ':' -f8)
    echo "$opcao.$origem para $destino, $data_partida, Partida:$hora_partida, Preço: $preco, Disponíveis:$lugares_disponiveis lugares"
    ((index2++))
done 

echo "0.Sair"

read -p "Insira o voo que pretende reservar: " escolha

voo_escolhido_codigo=${codigos_validos[escolha - 1]}

origem_escolhida=$(grep "$voo_escolhido_codigo" voos.txt | cut -d ':' -f2)
destino_escolhido=$(grep "$voo_escolhido_codigo" voos.txt | cut -d ':' -f3)
data_partida_escolhida=$(grep "$voo_escolhido_codigo" voos.txt | cut -d ':' -f4)
hora_partida_escolhida=$(grep "$voo_escolhido_codigo" voos.txt | cut -d ':' -f5)
preco_escolhido=$(grep "$voo_escolhido_codigo" voos.txt | cut -d ':' -f6)
lugares_disponiveis_escolhido=$(grep "$voo_escolhido_codigo" voos.txt | cut -d ':' -f8)


if [ "$escolha" -gt 0 ] && [ "$escolha" -lt $((1 + length2)) ]; then
    so_success S2.1.3 "$origem_escolhida para $destino_escolhido, $data_partida_escolhida, Partida:$hora_partida_escolhida, Preço: $preco_escolhido, Disponíveis:$lugares_disponiveis_escolhido lugares"
else
    so_error S2.1.3
    exit 1
fi

## S2.1.4. O programa pede ao utilizador para inserir uma opção da classe que pretende viajar: Lista as classes disponíveis para o voo selecionado. O preço de cada classe é calculado da seguinte forma: ❖ Preço da Classe Económica (ECONOMIC) = Preço Base ❖ Preço da Classe Executiva (EXECUTIVE) = Preço Base * 2 (o dobro do Preço Base) ❖ Preço da Primeira Classe (FIRST) = Preço Base *3 (o triplo do Preço Base). O utilizador insere a opção da classe do voo (neste exemplo, números de 1 a 3 ou 0). Se o utilizador escolheu um número de entre as opções de voo apresentadas (neste caso, entre 1 e 3), dá so_success <opção>. Caso contrário, dá so_error e termina.

read -p "Insira o ID do seu utilizador: " user_inserido

if grep -q "$user_inserido" passageiros.txt; then
    so_success S2.1.4
else
    so_error S2.1.4
    exit 1
fi

## S2.1.5. O programa pede ao utilizador o seu <ID_passageiro>: O utilizador insere o respetivo ID de passageiro (dica: UserId Linux). Caso o script veja que esse número corresponda a um utilizador não registado no ficheiro passageiros.txt, dá so_error e termina. Caso contrário, reporta so_success <ID_passageiro>.

read -p "Insira a senha do seu utilizador: " senha_inserida

senha_comparar=$(grep "$user_inserido" passageiros.txt | cut -d ':' -f5)

if [ "$senha_inserida" == "$senha_comparar" ]; then
    so_success S2.1.5
else
    so_error S2.1.5
    exit 1
fi

## S2.2. Processamento da resposta:
## S2.2.1. Valida se o passageiro possui <Saldo>, definido no ficheiro passageiros.txt, para comprar o bilhete selecionado no passo S2.1.4. Se a compra não é possível por falta de saldo, dá so_error <preço voo> <Saldo> e termina. Caso contrário, dá so_success <preço voo> <Saldo>.

saldo_disponivel=$(grep "$user_inserido" passageiros.txt | cut -d ':' -f6)

if [ "$saldo_disponivel" -lt "$preco_escolhido" ]; then
    so_error S2.2.1 "$preco_escolhido" "$saldo_disponivel"
    exit 1
else
    so_success S2.2.1 "$preco_escolhido" "$saldo_disponivel"
fi

## S2.2.2. Subtrai o valor do <preço voo> no <Saldo> do passageiro, e atualiza o ficheiro passageiros.txt. Em caso de erro (e.g., na escrita do ficheiro), dá so_error e termina. Senão, dá so_success <Saldo Atual>.

saldo_atualizado=$(("$saldo_disponivel" - "$preco_escolhido" ))

sed -i "/$user_inserido:/ s/:[^:]*$/:$saldo_atualizado/" passageiros.txt

if [ $? -eq 0 ]; then
    so_success S2.2.2 "$saldo_atualizado"
else
    so_error S2.2.2
    exit 1
fi

## S2.2.3. Decrementa uma unidade aos lugares disponíveis do voo e da classe escolhidos no passo S2.1.4, verificando se é preciso atualizar o <Status> desse voo, e atualiza o ficheiro estado_voos.txt. Em caso de erro (por exemplo, na escrita do ficheiro), dá so_error e termina. Senão, dá so_success <Status>.

lugares_atualizados=$(("$lugares_disponiveis_escolhido" - 1 ))

sed -i "/$voo_escolhido_codigo:/ s/:[^:]*$/:$lugares_atualizados/" voos.txt

if [ $? -eq 0 ]; then
    so_success S2.2.3
else
    so_error S2.2.2
    exit 1
fi

## S2.2.4. Regista a compra no ficheiro relatorio_reservas.txt, inserido uma nova linha no final deste ficheiro. Em caso de erro (por exemplo, na escrita do ficheiro), dá so_error e termina. Caso contrário, dá so_success.

ultima_reserva=$(tail -n 1 relatorio_reservas.txt)
id_reserva=$(grep "$ultima_reserva" relatorio_reservas.txt | cut -d ':' -f1)
id_reserva_atualizado=$(( $id_reserva + 1 ))

data_reserva=$(date +"%Y-%m-%d")
hora_reserva=$(date +"%Hh%M")

reserva="$id_reserva_atualizado:$voo_escolhido_codigo:$origem_escolhida:$destino_escolhido:$preco_escolhido:$user_inserido:$data_reserva:$hora_reserva"

echo "$reserva" >> relatorio_reservas.txt

if [ $? -eq 0 ] && [ -e relatorio_reservas.txt ]; then
    so_success S2.2.4
else
    so_error S2.2.4
    exit 1
fi