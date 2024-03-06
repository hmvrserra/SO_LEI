#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024
##
## Aluno: Nº: 105054 Nome: Hugo Magalhães do Vale Rosa Serra
## Nome do Módulo: S1. Script: regista_passageiro.sh
## Descrição/Explicação do Módulo: 
##
##
###############################################################################

## S1.1. Valida os argumentos passados e os seus formatos:
## S1.1.1. Valida os argumentos passados, avaliando se são em número suficiente (mínimo 3, máximo 4). Em caso de erro, dá so_error S1.1.1 e termina. Caso contrário, dá so_success S1.1.1.

if [ "$#" -lt 3 ] || [ "$#" -gt 4 ]; then
    so_error S1.1.1
    exit 1
else
    so_success S1.1.1
fi

## S1.1.2. Valida se o argumento <Nome> corresponde ao nome de um utilizador do servidor Tigre. Se não corresponder ao nome de nenhum utilizador do Tigre, dá so_error S1.1.2 e termina. Senão, dá so_success S1.1.2.

nome="$1"
senha="$2"

if ! grep -w -q "$nome" _etc_passwd; then
    so_error S1.1.2
    exit 1
else
    so_success S1.1.2
fi

## S1.1.3. Valida se o argumento <Saldo a adicionar> tem formato “number” (inteiro positivo ou 0). Se não tiver, dá so_error S1.1.3 e termina. Caso contrário, dá so_success S1.1.3.

saldo="$3"

if ! [[ $saldo =~ [0-9]+$ ]]; then
    so_error S1.1.3
    exit 1
else
    so_success S1.1.3
fi

## S1.1.4. Valida se o argumento opcional <NIF> (só no caso de ser passado, i.e., se tiver valor) tem formato “number” com 9 (nove) dígitos. Se não for, dá so_error S1.1.4 e termina. Caso contrário, dá so_success S1.1.4.

nif="$4"

if [ -n "$nif" ]; then
    if [[ $nif =~ ^[0-9]{9}$ ]]; then
        so_success S1.1.4
    else
        so_error S1.1.4
        exit 1
    fi
else
    so_success S1.1.4
fi


## S1.2. Associa os dados passados com a base de dados dos passageiros registados:
## S1.2.1. Verifica se o ficheiro passageiros.txt existe. Se o ficheiro existir, dá so_success S1.2.1 e continua no passo S1.2.3. Se não existir, dá so_error S1.2.1, e continua.

if [ -e passageiros.txt ]; then
    so_success S1.2.1
    skip_S1_2_2=true
else
    so_error S1.2.1
    skip_S1_2_2=false
fi

## S1.2.2. Cria o ficheiro passageiros.txt. Se der erro, dá so_error S1.2.2 e termina. Senão, dá so_success S1.2.2.

#Cria o ficheiro passageiros.txt
if [ "$skip_S1_2_2" = false ]; then
    touch passageiros.txt

    #Verifica se o ficheiro foi criado corretamente
    if [ ! $? -eq 0 ]; then
        so_error S1.2.2
        exit 1
    else
        so_success S1.2.2
    fi
fi

## S1.2.3. Caso o passageiro <Nome> passado já exista no ficheiro passageiros.txt, dá so_success S1.2.3, e continua no passo S1.3. Senão, dá so_error S1.2.3, e continua.

if cat passageiros.txt | grep -q "$nome"; then
    so_success S1.2.3
    skip_S1_2_x=true
else
    so_error S1.2.3
    skip_S1_2_x=false
fi

## S1.2.4. Como o passageiro <Nome> não existe no ficheiro, terá de o registar. Para isso, valida se <NIF> (campo opcional) foi mesmo passado. Se não foi, dá so_error S1.2.4 e termina. Senão, dá so_success S1.2.4.

if [ "$skip_S1_2_x" = false ]; then
    if [ ! -n "$nif" ]; then
        so_error S1.2.4
        exit 1
    else
        so_success S1.2.4
    fi

## S1.2.5. Define o campo <ID_passageiro>, como sendo o UserId Linux associado ao utilizador de nome <Nome> no servidor Tigre. Em caso de haver algum erro na operação, dá so_error S1.2.5 e termina. Caso contrário, dá so_success S1.2.5 <ID_passageiro> (substituindo pelo campo definido).
    
    #Atribui o user_id a uma variável id_passageiro
    id_passageiro=$(cat _etc_passwd | grep -m 1 "$nome" | cut -d ':' -f1)

    #Verifica se o id_passageiro é uma string vazia ou não para aferir o sucesso da operação
    if [ -z "$id_passageiro" ]; then
        so_error S1.2.5
        exit 1
    else
        so_success S1.2.5 "$id_passageiro"
    fi

## S1.2.6. Define o campo <Email>, gerado a partir do <Nome> introduzido pelo utilizador, usando apenas o primeiro e o último nome, convertendo-os para minúsculas apenas, colocando um ponto entre os dois nomes, e domínio iscteflight.pt. Assim sendo, um exemplo seria “david.gabriel@iscteflight.pt”. Se houver algum erro na operação (e.g., o utilizador “root” tem menos de 2 nomes), dá so_error S1.2.6 e termina. Caso contrário, dá so_success S1.2.6 <Email> (substituindo pelo campo gerado). Ao registar um novo passageiro no sistema, o número inicial de <Saldo> tem o valor 0 (zero).

    #Atribui o email do usuário a uma variável email e cria duas variaveis para guardar o primeiro e ultimo nome de um usuario
    primeiro_nome=$(echo "$nome" | awk '{print $1}')
    ultimo_nome=$(echo "$nome" | awk '{print $NF}')

    email=$(echo "$primeiro_nome" "$ultimo_nome" | tr '[:upper:]' '[:lower:]' | awk '{gsub(" ", ".")}1')@iscteflight.pt

    #Verificar se o utlizador "root" tem menos de 2 nomes
    if [[ ! "$nome" =~ [[:space:]] ]]; then
        so_error S1.2.6
        exit 1
    else
        so_success S1.2.6 "$email"
    fi

## S1.2.7. Regista o utilizador numa nova linha no final do ficheiro passageiros.txt, seguindo a sintaxe: <ID_passageiro>:<NIF>:<Nome>:<Email>:<Senha>:<Saldo>. Em caso de haver algum erro na operação (e.g., erro na escrita do ficheiro), dá so_error S1.2.7 e termina. Caso contrário, dá so_success S1.2.7 <linha> (substituindo pela linha completa escrita no ficheiro).

    #Variavel que guardará o registro com a seguinte sintaxe <ID_passageiro>:<NIF>:<Nome>:<Email>:<Senha>:<Saldo>
    registro="$id_passageiro:$nif:$nome:$email:$senha:$saldo"

    #Adiciona o registro ao ficheiro passageiros.txt
    echo "$registro" >> passageiros.txt

    #Verifica se a adição foi bem-sucedida
    if [ $? -eq 0 ]; then
        so_success S1.2.7 "$registro"
    else
        so_error S1.2.7
        exit 1
    fi
fi

## S1.3. Adiciona créditos na conta de um passageiro que existe no ficheiro passageiros.txt:
## S1.3.1. Tendo já encontrado um “match” passageiro com o Nome <Nome> no ficheiro, valida se o campo <Senha> passado corresponde à senha registada no ficheiro. Se não corresponder, dá so_error S1.3.1 e termina. Caso contrário, dá so_success S1.3.1.

#Vai ao ficheiro passageiros.txt e extrai a senha guardada no mesmo
senha_registada=$(grep "$nome" passageiros.txt | cut -d ':' -f5)

#Compara a senha introduzida com a guardada no ficheiro passageiros.txt e se forem diferentes dá so_erros S1.3.1 e termina
if [ "$senha" != "$senha_registada" ]; then
    so_error S1.3.1
    exit 1
else
    so_success S1.3.1
fi

## S1.3.2. Mesmo que tenha sido passado um campo <NIF> (opcional), ignora-o. Adiciona o valor passado do campo <Saldo a adicionar> ao valor do <Saldo> registado no ficheiro passageiros.txt para o passageiro em questão, atualizando esse valor no ficheiro passageiros.txt. Se houver algum erro na operação (e.g., erro na escrita do ficheiro), dá so_error S1.3.2 e termina. Caso tudo tenha corrido bem, dá o resultado so_success S1.3.2 <Saldo> (substituindo pelo valor saldo atualizado no ficheiro passageiros.txt).

# Obtém o saldo atual do usuário através do arquivo passageiros.txt
saldo_atual=$(grep "$nome" passageiros.txt | cut -d ':' -f6)

# Obtém o saldo novo através do saldo anterior somado ao saldo introduzido
saldo_novo=$((saldo_atual + saldo))

# Atualiza o saldo no arquivo passageiros.txt se não for a primeira vez a registar o meso
if [ "$skip_S1_2_x" = true ]; then
    sed -i "/$nome:/ s/:[^:]*$/:$saldo_novo/" passageiros.txt

    # Verifica se houve algum erro na operação
    if [ $? -eq 0 ]; then
        so_success S1.3.2 "$saldo_novo"
    else
        so_error S1.3.2
        exit 1
    fi
fi
## S1.4. Lista todos os passageiros registados, mas ordenados por saldo:
## S1.4.1. O script deve criar um ficheiro chamado passageiros-saldos-ordenados.txt igual ao que está no ficheiro passageiros.txt, com a mesma formatação, mas com os registos ordenados por ordem decrescente do campo <Saldo> dos passageiros. Se houver algum erro (e.g., erro na leitura ou escrita do ficheiro), dá so_error S1.4.1, e termina. Caso contrário, dá so_success S1.4.1.

# Ordena os registros por ordem decrescente do campo <Saldo> e escreve no arquivo passageiros-saldos-ordenados.txt
sort -t ':' -k6,6nr passageiros.txt > passageiros-saldos-ordenados.txt

# Verifica se houve algum erro na operação
if [ $? -eq 0 ]; then
    so_success S1.4.1
else
    so_error S1.4.1
    exit 1
fi