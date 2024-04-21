/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2023/2024, Enunciado Versão 3+
 **
 ** Aluno: Nº: 105054  Nome: Hugo Magalhães do Vale Rosa Serra
 ** Nome do Módulo: cliente.c
 ** Descrição/Explicação do Módulo:
 **
 ** Este script tem como objetivo implementar um cliente para um sistema de check-in online de uma companhia aérea. 
 ** Ele solicita ao usuário o número de identificação fiscal (NIF) e uma senha, envia esses dados para 
 ** o servidor e aguarda uma resposta. O script usa sinais para lidar com diferentes situações, como 
 ** sucesso, falha, interrupção ou timeout.
 ******************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/**
 * @brief Processamento do processo Cliente
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main () {
    // C1
    checkExistsFifoServidor_C1(FILE_REQUESTS);
    // C2
    triggerSignals_C2();
    // C3 + C4
    CheckIn clientRequest = getDadosPedidoUtilizador_C3_C4();
    // C5
    writeRequest_C5(clientRequest, FILE_REQUESTS);
    // C6
    configureTimer_C6(MAX_ESPERA);
    // C7
    waitForEvents_C7();
    so_exit_on_error(-1, "ERRO: O cliente nunca devia chegar a este ponto");
}

/**
 *  "O módulo Cliente é responsável pela interação com o utilizador.
 *   Após o login do utilizador, este poderá realizar atividades durante o tempo da sessão.
 *   Assim, definem-se as seguintes tarefas a desenvolver:"
 */

/**
 * @brief C1       Ler a descrição da tarefa C1 no enunciado
 * @param nameFifo Nome do FIFO servidor (i.e., FILE_REQUESTS)
 */
void checkExistsFifoServidor_C1 (char *nameFifo) {
    so_debug("< [@param nameFifo:%s]", nameFifo);

    struct stat isFifo;

    if(access(nameFifo, F_OK) == -1) { //Verifica se o FIFO existe
        so_error("C1", "");
        exit(1);
    }else if (stat(nameFifo, &isFifo) == -1 || !S_ISFIFO(isFifo.st_mode)) { //Verifica se o FIFO é um FIFO
        so_error("C1", "");
        exit(1);
    }
    so_success("C1", "");

    so_debug(">");
}

/**
 * @brief C2   Ler a descrição da tarefa C2 no enunciado
 */
void triggerSignals_C2 () {
    so_debug("<");

    if (signal(SIGUSR1, trataSinalSIGUSR1_C8) == SIG_ERR){ //Arma e trata o sinal SIGUSR1
        so_error("C2", "");
        exit(1);
    }else if (signal(SIGHUP, trataSinalSIGHUP_C9) == SIG_ERR){ //Arma e trata o sinal SIGHUP
        so_error("C2", "");
        exit(1);
    }else if (signal(SIGINT, trataSinalSIGINT_C10) == SIG_ERR){ //Arma e trata o sinal SIGINT
        so_error("C2", "");
        exit(1);
    }else if (signal(SIGALRM, trataSinalSIGALRM_C11) == SIG_ERR){ //Arma e trata o sinal SIGALRM
        so_error("C2", "");
        exit(1);
    }
    so_success("C2", "");

    so_debug(">");
}

/**
 * @brief C3+C4    Ler a descrição das tarefas C3 e C4 no enunciado
 * @return CheckIn Elemento com os dados preenchidos. Se nif=-1, significa que o elemento é inválido
 */
CheckIn getDadosPedidoUtilizador_C3_C4 () {
    CheckIn request;
    request.nif = -1;   // Por omissão retorna erro
    so_debug("<");

    printf("IscteFlight: Check-In Online \n");  //Apresentação do programa
    printf("---------------------------- \n");
    printf("Introduza o NIF do passageiro: ");
    scanf("%d", &request.nif);

    if (request.nif < 0 || request.nif > 999999999){ //Verifica se o NIF é válido (9 digitos)
        so_error("C3", "");
        exit(1);
    }
    
    printf("\nIntroduza a senha do passageiro: "); //Introdução da senha
    scanf("%s", request.senha);

    request.pidCliente = getpid();  //Preenche o PID do Cliente

    so_success("C4", "%d %s %d", request.nif, request.senha, request.pidCliente); 

    so_debug("> [@return nif:%d, senha:%s, pidCliente:%d]", request.nif, request.senha, request.pidCliente);
    return request;
}
 
void writeRequest_C5 (CheckIn request, char *nameFifo) {
    so_debug("< [@param request.nif:%d, request.senha:%s, request.pidCliente:%d, nameFifo:%s]",
                                        request.nif, request.senha, request.pidCliente, nameFifo);

    FILE *fifo = fopen(nameFifo, "w"); //Abre o FIFO para escrita
    if (fifo == NULL){  //Verifica se o FIFO foi aberto corretamente
        so_error("C5", "");
        exit(1);
    }

    if (fprintf(fifo, "%d\n", request.nif) < 0){ //Escreve o NIF no FIFO
        so_error("C5", "");
        exit(1);
    }else if (fprintf(fifo, "%s\n", request.senha) < 0){ //Escreve a senha no FIFO
        so_error("C5", "");
        exit(1);
    }else if (fprintf(fifo, "%d\n", request.pidCliente) < 0){ //Escreve o PID do Cliente no FIFO
        so_error("C5", "");
        exit(1);
    }
    fclose(fifo);   //Fecha o FIFO
    so_success("C5", "");
    

    so_debug(">");
}

/**
 * @brief C6          Ler a descrição da tarefa C6 no enunciado
 * @param tempoEspera o tempo em segundos que queremos pedir para marcar o timer do SO (i.e., MAX_ESPERA)
 */
void configureTimer_C6 (int tempoEspera) {
    so_debug("< [@param tempoEspera:%d]", tempoEspera);

    alarm(tempoEspera); //Marca o timer do SO
    so_success("C6", "Espera resposta em %d segundos", tempoEspera);

    so_debug(">");
}

/**
 * @brief C7 Ler a descrição da tarefa C7 no enunciado
 */
void waitForEvents_C7 () {
    so_debug("<");

    pause();    //Pausa o processo até receber um sinal

    so_debug(">");
}

/** @brief C8 Ler a descrição da tarefa C8 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR1_C8 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    if (sinalRecebido == SIGUSR1){  //Verifica se o sinal recebido é o SIGUSR1
        so_success("C8", "Check-In concluído com sucesso");
        exit(0);
    }

    so_debug(">");
}

/**
 * @brief C9            Ler a descrição da tarefa C9 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGHUP_C9 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    if (sinalRecebido == SIGHUP){   //Verifica se o sinal recebido é o SIGHUP
        so_success("C9", "Check-In concluído sem sucesso");
        exit(1);
    }

    so_debug(">");
}

/**
 * @brief C10           Ler a descrição da tarefa C10 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_C10 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    if (sinalRecebido == SIGINT){   //Verifica se o sinal recebido é o SIGINT
        so_success("C10", "Cliente: Shutdown");
        exit(0);
    }

    so_debug(">");
}

/**
 * @brief C11           Ler a descrição da tarefa C11 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGALRM_C11 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_error("C11", "Cliente: Timeout");    //Timeout
    exit(1);

    so_debug(">");
}
