#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024
##
## Aluno: Nº: 105054 Nome: Hugo Magalhães do vale Rosa Serra
## Nome do Módulo: S3. Script: estado_voos.sh
## Descrição/Explicação do Módulo:
##
##
###############################################################################

## S3.1. Validações:
## S3.1.1. O script valida se os ficheiros voos.txt e estado_voos.txt existem. Se algum deles não existir, dá so_error e termina. Caso contrário, dá so_success.

if [[ ! -f ./voos.txt ]]; then
    so_error S3.1.1
    exit 1
else
    so_success S3.1.1
fi

## S3.1.2. O script valida se os formatos de todos os campos de cada linha do ficheiro voos.txt correspondem à especificação indicada em S2, nomeadamente se respeitam os formatos de data e de hora. Se alguma linha não respeitar, dá so_error <conteúdo da linha> e termina. Caso contrário, dá so_success.


while read linha; do

    codigo_ver=$(grep "$linha" voos.txt | cut -d ':' -f1)
    origem_ver=$(grep "$linha" voos.txt | cut -d ':' -f2)
    destino_ver=$(grep "$linha" voos.txt | cut -d ':' -f3)
    data_partida_ver=$(grep "$linha" voos.txt | cut -d ':' -f4)
    hora_partida_ver=$(grep "$linha" voos.txt | cut -d ':' -f5)
    preco_ver=$(grep "$linha" voos.txt | cut -d ':' -f6)
    lotacao_ver=$(grep "$linha" voos.txt | cut -d ':' -f7)
    lugares_disponiveis_ver=$(grep "$linha" voos.txt | cut -d ':' -f8)

    if [[ -z $codigo_ver || ! $codigo_ver =~ ^[A-Z]{2}[0-9]{4}$ ]]; then
        so_error S3.1.2 "$linha"
        exit
    elif [[ -z $origem_ver || ! $origem_ver =~ [[:alnum:]]+[[:space:]]*[[:alnum:]]* ]]; then
        so_error S3.1.2 "$linha"
        exit
    elif [[ -z $destino_ver || ! $destino_ver =~ [[:alnum:]]+[[:space:]]*[[:alnum:]]* ]]; then
        so_error S3.1.2 "$linha"
        exit
    elif [[ -z $data_partida_ver || ! $data_partida_ver =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]]; then
        so_error S3.1.2 "$linha"
        exit
    elif [[ -z $hora_partida_ver || ! $hora_partida_ver =~ ^([01][0-9]|2[0-3])h[0-5][0-9]$ ]]; then
        so_error S3.1.2 "$linha"
        exit
    elif [[ -z $preco_ver || ! $preco_ver =~ ^[0-9]+$ ]]; then
        so_error S3.1.2 "$linha"
        exit
    elif [[ -z $lotacao_ver || ! $lotacao_ver =~ ^[0-9]+$ ]]; then
        so_error S3.1.2 "$linha"
        exit
    elif [[ -z $lugares_disponiveis_ver || ! $lugares_disponiveis_ver =~ ^[0-9]+$ ]]; then
        so_error S3.1.2 "$linha"
        exit
    fi


done < voos.txt

so_success S3.1.2

## S3.2. Processamento do script:
## S3.2.1. O script deverá ler cada linha do ficheiro voos.txt. Se o ficheiro estado_voos.txt já tiver alguma linha com o <NrVoo>, então dá so_success e avança para S3.2.3. Caso contrário, dá so_error e avança.
touch voos_disponiveis.html

data_atual=$(date +"%Y-%m-%d")
hora_atual=$(date +"%Hh%M")

codigo_voo=$(grep ":" voos.txt | awk -F: '$8 != 0 { print $1 }')
readarray -t codigos <<< "$codigo_voo"

index=0
length=${#codigos[@]}

echo "<html><head><meta charset=\"UTF-8\"><title>IscteFlight: Lista de Voos Disponíveis</title></head>" > voos_disponiveis.html
echo "<body><h1>Lista atualizada em $data_atual $hora_atual</h1>" >> voos_disponiveis.html

while [ $index -lt $length ]; do
    voo_info=$(grep "${codigos[$index]}" voos.txt)
    if [ -n "$voo_info" ]; then
        origem=$(echo "$voo_info" | cut -d ':' -f2)
        destino=$(echo "$voo_info" | cut -d ':' -f3)
        data_partida=$(echo "$voo_info" | cut -d ':' -f4)
        hora_partida=$(echo "$voo_info" | cut -d ':' -f5)
        preco=$(echo "$voo_info" | cut -d ':' -f6)
        lotacao=$(echo "$voo_info" | cut -d ':' -f7)
        lugares_disponiveis=$(echo "$voo_info" | cut -d ':' -f8)
        lugares_ocupados=$(("$lotacao" - "$lugares_disponiveis"))

        echo "<h2>Voo: ${codigos[$index]}, De: $origem Para: $destino, Partida em $data_partida $hora_partida</h2>" >> voos_disponiveis.html
        echo "<ul>" >> voos_disponiveis.html
        echo "<li><b>Lotação:</b> $lotacao Lugares</li>" >> voos_disponiveis.html
        echo "<li><b>Lugares Disponíveis:</b> $lugares_disponiveis Lugares</li>" >> voos_disponiveis.html
        echo "<li><b>Lugares Ocupados:</b> $lugares_ocupados Lugares</li>" >> voos_disponiveis.html
        echo "</ul>" >> voos_disponiveis.html
    fi
    ((index++))
done

echo "</body></html>" >> voos_disponiveis.html

if [ $? -eq 0 ]; then
    so_success S3.2.1
else
    so_error S3.2.1
fi

## S3.3. Invocação do script estado_voos.sh:
## S3.3.1. Altere o ficheiro cron.def fornecido, por forma a configurar o seu sistema para que o script seja executado de hora em hora, mas apenas nos dias ímpares. Nos comentários no início do ficheiro cron.def, explique a configuração realizada, e indique qual o comando que deveria utilizar para despoletar essa configuração. O ficheiro cron.def alterado deverá ser submetido para avaliação juntamente com os outros Shell scripts.

###############FAZEEEEEEEEEEEEEEEEEEEEEEEER