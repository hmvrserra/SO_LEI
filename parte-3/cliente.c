/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos 2023/2024, Enunciado Versão 1+
 **
 ** Aluno: Nº: 105054   Nome: Hugo Magalhães do Vale Rosa Serra
 ** Nome do Módulo: cliente.c
 ** Descrição/Explicação do Módulo:
 **
 ** O módulo Cliente interage com o utilizador durante a sessão. Ele abre a Message Queue, arma sinais 
 ** SIGHUP, SIGINT e SIGALRM, e coleta dados de autenticação. Envia a mensagem de login para a 
 ** Message Queue e espera a resposta do servidor. Ao receber a resposta, desliga o alarme, verifica a
 ** validade do pidServidorDedicado, e confirma a reserva. Se necessário, pede ao utilizador para 
 ** escolher um novo lugar e envia essa escolha. O cliente trata sinais: SIGHUP (servidor vai terminar),
 ** SIGINT (cancelar pedido), e SIGALRM (tempo de espera excedido). Termina validando o 
 ** pidServidorDedicado e enviando um sinal SIGUSR1, se aplicável.
 ******************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "defines.h"

/*** Variáveis Globais ***/
int msgId;                              // Variável que tem o ID da Message Queue
MsgContent clientRequest;               // Variável que serve para as mensagens trocadas entre Cliente e Servidor
int primeiraTentativa = 1;              // Variável que serve para verificar se é a primeira tentativa de reserva de lugar

/**
 * @brief Processamento do processo Cliente
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main () {
    // C1
    msgId = initMsg_C1();
    so_exit_on_error(msgId, "initMsg_C1");
    // C2
    so_exit_on_error(triggerSignals_C2(), "triggerSignals_C2");
    // C3
    so_exit_on_error(getDadosPedidoUtilizador_C3(), "getDadosPedidoUtilizador_C3");
    // C4
    so_exit_on_error(sendRequest_C4(), "sendRequest_C4");
    // C5: CICLO6
    while (TRUE) {
        // C5
        configureTimer_C5(MAX_ESPERA);
        // C6
        so_exit_on_error(readResponseSD_C6(), "readResponseSD_C6");
        // C7
        int lugarEscolhido = trataResponseSD_C7();
        if (RETURN_ERROR == lugarEscolhido)
            terminateCliente_C9();
        // C8
        if (RETURN_ERROR == sendSeatChoice_C8(lugarEscolhido))
            terminateCliente_C9();
    }
}

/**
 *  "O módulo Cliente é responsável pela interação com o utilizador.
 *   Após o login do utilizador, este poderá realizar atividades durante o tempo da sessão.
 *   Assim, definem-se as seguintes tarefas a desenvolver:"
 */

/**
 * @brief C1: Ler a descrição da tarefa no enunciado
 * @return o valor de msgId em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int initMsg_C1 () {
    msgId = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    msgId = msgget(IPC_KEY, 0); // Abre a Message Queue
    
    if (msgId == -1) { // Verifica se a Message Queue foi aberta com sucesso
        so_error("C1", "");
        return RETURN_ERROR;
    }

    so_success("C1", "%d", msgId);
    return RETURN_SUCCESS;
}

/**
 * @brief C2: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int triggerSignals_C2 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    if (signal(SIGHUP, trataSinalSIGHUP_C10) == SIG_ERR) { // Verifica se o sinal SIGHUP foi recebido com sucesso
        so_error("C2", "");
        return RETURN_ERROR;
    }

    if (signal(SIGINT, trataSinalSIGINT_C11) == SIG_ERR) { // Verifica se o sinal SIGINT foi recebido com sucesso
        so_error("C2", "");
        return RETURN_ERROR;
    }

    if (signal(SIGALRM, trataSinalSIGALRM_C12) == SIG_ERR) { // Verifica se o sinal SIGALRM foi recebido com sucesso
        so_error("C2", "");
        return RETURN_ERROR;
    }

    so_success("C2", "");
    return RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C3: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int getDadosPedidoUtilizador_C3 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    int nif;
    char senha[20];

    printf("IscteFlight: Check-In Online \n");  //Apresentação do programa
    printf("---------------------------- \n");
    printf("Introduza o NIF do passageiro: ");
    scanf("%d", &nif);

    if (nif < 0 || nif > 999999999){ //Verifica se o NIF é válido (9 digitos)
        so_error("C3", "");
        return RETURN_ERROR;
    }
    
    printf("\nIntroduza a senha do passageiro: "); //Introdução da senha
    scanf("%s", senha);

    clientRequest.msgData.infoCheckIn.nif = nif;
    strcpy(clientRequest.msgData.infoCheckIn.senha, senha);
    so_success("C3", "");
    return RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C4: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int sendRequest_C4 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    clientRequest.msgType = MSGTYPE_LOGIN; 
    clientRequest.msgData.infoCheckIn.pidCliente = getpid();
    clientRequest.msgData.infoCheckIn.pidServidorDedicado = PID_INVALID;

    if (msgsnd(msgId, &clientRequest, sizeof(MsgContent) - sizeof(long), 0) == -1) { //Envia a mensagem para o Servidor
        so_error("C4", "");
        return RETURN_ERROR;
    }
    so_success("C4", "%d %s %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.senha, clientRequest.msgData.infoCheckIn.pidCliente); 
    return RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C5: Ler a descrição da tarefa no enunciado
 * @param tempoEspera o tempo em segundos que queremos pedir para marcar o timer do SO (i.e., MAX_ESPERA)
 */
void configureTimer_C5 (int tempoEspera) {
    so_debug("< [@param tempoEspera:%d]", tempoEspera);

    // C5
    alarm(tempoEspera); //Marca o alarme para o tempo de espera
    so_success("C5", "Espera resposta em %d segundos", tempoEspera); //Espera pela resposta do Servidor

    so_debug(">");
}

/**
 * @brief C6: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int readResponseSD_C6 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    if (msgrcv(msgId, &clientRequest, sizeof(MsgContent) - sizeof(long), getpid(), 0) == -1) { //Recebe a mensagem do Servidor
        so_error("C6", "");
        return RETURN_ERROR;
    }
    so_success("C6", "%d %d %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.lugarEscolhido, clientRequest.msgData.infoCheckIn.pidCliente);
    return RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C7: Ler a descrição da tarefa no enunciado
 * @return Nº do lugar escolhido (0..MAX_SEATS-1) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int trataResponseSD_C7 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    alarm(0); //C7.1

    if (clientRequest.msgData.infoCheckIn.pidServidorDedicado == PID_INVALID) { //C7.2
        so_error("C7.2", "");
        exit(1);
        terminateCliente_C9();
    }

    if (clientRequest.msgData.infoCheckIn.lugarEscolhido != EMPTY_SEAT) { //C7.3
        so_success("C7.3", "Reserva concluída: %s %s %d", clientRequest.msgData.infoVoo.origem, clientRequest.msgData.infoVoo.destino, clientRequest.msgData.infoCheckIn.lugarEscolhido);
        exit(0);
        terminateCliente_C9();
    }

    if (clientRequest.msgData.infoCheckIn.lugarEscolhido == EMPTY_SEAT) { //C7.4
        if (primeiraTentativa == 1) { //C7.4.1
            so_success("C7.4.1", "");
            primeiraTentativa = 0;
        } else {
            so_error("C7.4.1", "Erro na reserva de lugar");
        }
    }
    int i;  
    int lugaresDisponiveis = 0;

    printf("IscteFlight: Voo %s\n", clientRequest.msgData.infoCheckIn.nrVoo); //C7.4.2
    printf("---------------------------- \n");
    printf("Lugares disponíveis: ");

    for (i = 0; i < MAX_SEATS; i++) {
        if (clientRequest.msgData.infoVoo.lugares[i] == EMPTY_SEAT) {
            lugaresDisponiveis++;
        }
    }

    for (i = 0; i < MAX_SEATS; i++) {
        if (clientRequest.msgData.infoVoo.lugares[i] == EMPTY_SEAT) {
        printf("%d", i);
            lugaresDisponiveis--;
            if (lugaresDisponiveis > 0) {
            printf(", ");
            }
        }
    }

    printf("\nIntroduza o lugar que deseja reservar: ");
    scanf("%d", &result);
    if (result < 0 || result > (MAX_SEATS-1)) {
        so_error("C7.4.3", "");
        return RETURN_ERROR;
    } else {
        so_success("C7.4.3", "%d", result);
    }
    primeiraTentativa = 1;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C8: Ler a descrição da tarefa no enunciado
 * @param lugarEscolhido índice do array lugares que o utilizador escolheu, entre 0 e MAX_SEATS-1
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int sendSeatChoice_C8 (int lugarEscolhido) {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("< [@param lugarEscolhido:%d]", lugarEscolhido);

    clientRequest.msgType = clientRequest.msgData.infoCheckIn.pidServidorDedicado;
    clientRequest.msgData.infoCheckIn.lugarEscolhido = lugarEscolhido;
    clientRequest.msgData.infoCheckIn.pidCliente = getpid();

    if (msgsnd(msgId, &clientRequest, sizeof(MsgContent) - sizeof(long), 0) == -1) { //Envia a mensagem para o Servidor
        so_error("C8", "");
        return RETURN_ERROR;
    }

    so_success("C8", "%d %d %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.lugarEscolhido, clientRequest.msgData.infoCheckIn.pidCliente);
    return RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C9: Ler a descrição da tarefa no enunciado
 */
void terminateCliente_C9 () {
    so_debug("<");

    if (clientRequest.msgData.infoCheckIn.pidServidorDedicado == PID_INVALID) { //Verifica se o Servidor está ativo
        so_error("C9", "");
        exit(1);
    }
    so_success("C9", "");

    if (kill(clientRequest.msgData.infoCheckIn.pidServidorDedicado, SIGUSR1) == -1){ //Envia sinal para o Servidor
        so_error("C9", "");
        exit(1);
    }

    so_debug(">");
    exit(0);
}

/**
 * @brief C10: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGHUP_C10 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("C10", "Check-in concluído sem sucesso"); //Sinal recebido
    exit(0);

    so_debug(">");
}

/**
 * @brief C11: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_C11 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido); 

    so_success("C11", "Cliente: Shutdown"); //Sinal recebido
    terminateCliente_C9();

    so_debug(">");
}

/**
 * @brief C12: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGALRM_C12 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_error("C12", "Cliente: Timeout"); //Sinal recebido
    terminateCliente_C9();

    so_debug(">");
}