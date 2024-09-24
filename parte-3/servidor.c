/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos 2023/2024, Enunciado Versão 1+
 **
 ** Aluno: Nº: 105054   Nome: Hugo Magalhães do Vale Rosa Serra
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo:
 **
 ** O servidor.c gerencia autenticação e voos, usando memória compartilhada, fila de mensagens e 
 ** semáforos. Processa logins, cria e encerra servidores dedicados, e atualiza dados de passageiros e 
 ** voos.
 ******************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "defines.h"

/*** Variáveis Globais ***/
int shmId;                              // Variável que tem o ID da Shared Memory
int msgId;                              // Variável que tem o ID da Message Queue
int semId;                              // Variável que tem o ID do Grupo de Semáforos
MsgContent clientRequest;               // Variável que serve para as mensagens trocadas entre Cliente e Servidor
DadosServidor *database = NULL;         // Variável que vai ficar com UM POINTER PARA a memória partilhada
int indexClient = -1;                   // Índice do passageiro que fez o pedido ao servidor/servidor dedicado na BD
int indexFlight = -1;                   // Índice do voo do passageiro que fez o pedido ao servidor/servidor dedicado na BD
int nrServidoresDedicados = 0;         // Número de servidores dedicados (só faz sentido no processo Servidor)

/**
 * @brief Processamento do processo Servidor e dos processos Servidor Dedicado
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main () {
    // S1
    shmId = initShm_S1();
    if (RETURN_ERROR == shmId) terminateServidor_S7();
    // S2
    msgId = initMsg_S2();
    if (RETURN_ERROR == msgId) terminateServidor_S7();
    // S3
    semId = initSem_S3();
    if (RETURN_ERROR == semId) terminateServidor_S7();
    // S4
    if (RETURN_ERROR == triggerSignals_S4()) terminateServidor_S7();

    // S5: CICLO1
    while (TRUE) {
        // S5
        int result = readRequest_S5();
        if (CICLO1_CONTINUE == result) // S5: "Se receber um sinal (...) retorna o valor CICLO1_CONTINUE"
            continue;                  // S5: "para que main() recomece automaticamente o CICLO1 no passo S5"
        if (RETURN_ERROR == result) terminateServidor_S7();
        // S6
        int pidServidorDedicado = createServidorDedicado_S6();
        if (pidServidorDedicado > 0)   // S6: "o processo Servidor (pai) (...) retorna um valor > 0"
            continue;                  // S6: "(...) recomeça o Ciclo1 no passo S4 (ou seja, volta a aguardar novo pedido)"
        if (RETURN_ERROR == pidServidorDedicado) terminateServidor_S7();
        // S6: "o Servidor Dedicado (...) retorna 0 para que main() siga automaticamente para o passo SD10

        // SD10
        if (RETURN_ERROR == triggerSignals_SD10()) terminateServidorDedicado_SD18();
        // SD11
        indexClient = searchClientDB_SD11();
        int erroValidacoes = TRUE;
        if (RETURN_ERROR != indexClient) {
            // SD12: "Se o passo SD11 concluiu com sucesso: (...)"
            indexFlight = searchFlightDB_SD12();
            if (RETURN_ERROR != indexFlight) {
                // SD13: "Se os passos SD11 e SD12 tiveram sucesso, (...)"
                if (!updateClientDB_SD13())
                    erroValidacoes = FALSE; // erroValidacoes = "houve qualquer erro nas validações dos passos SD11, SD12, ou SD13"
            }
        }
        // SD14: CICLO5
        int escolheuLugarDisponivel = FALSE;
        while (!escolheuLugarDisponivel) {
            // SD14.1: erroValidacoes = "houve qualquer erro nas validações dos passos SD11, SD12, ou SD13"
            if (RETURN_ERROR == sendResponseClient_SD14(erroValidacoes)) terminateServidorDedicado_SD18();
            if (erroValidacoes)
                terminateServidorDedicado_SD18();

            // SD15: "Se os pontos anteriores tiveram sucesso, (...)"
            if (RETURN_ERROR == readResponseClient_SD15()) terminateServidorDedicado_SD18();
            // SD16
            if (RETURN_ERROR == updateFlightDB_SD16())  // SD16: "Se lugarEscolhido no pedido NÃO estiver disponível (...) retorna erro (-1)"
                continue;                               // SD16: "para que main() recomece o CICLO5 em SD14"
            else
                escolheuLugarDisponivel = TRUE;
        }
        sendConfirmationClient_SD17();
        terminateServidorDedicado_SD18();
    }
}
/**
 *  "O módulo Servidor é responsável pelo processamento do check-in dos passageiros.
 *   Está dividido em duas partes, um Servidor (pai) e zero ou mais Servidores Dedicados (filhos).
 *   Este módulo realiza as seguintes tarefas:"
 */

/**
 * @brief S1: Ler a descrição da tarefa no enunciado
 * @return o valor de shmId em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int initShm_S1 () {
    shmId = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // S1.1
    if (access("bd_passageiros.dat", R_OK | W_OK) == -1 || access("bd_voos.dat", R_OK | W_OK) == -1) {  // Verifica se os ficheiros existem e têm permissões de leitura e escrita
        so_error("S1.1", "");
        return RETURN_ERROR;
    }
    so_success("S1.1", "");

    // S1.2
    int tempId = shmget(IPC_KEY, 0, 0); // Verifica se a Shared Memory já existe
    if (tempId != -1) { // Se existir, liga-se a ela
        so_success("S1.2", "");
        database = (DadosServidor *) shmat(tempId, NULL, 0);    // Liga-se à Shared Memory
        if (database == NULL) { // Se não conseguir ligar-se, retorna erro
            so_error("S1.2.1", "");
            return RETURN_ERROR;
        } else {
            shmId = tempId; 
            so_success ("S1.2.1", "%d", shmId);
            return shmId;
        }
    } else {
        so_error("S1.2", "");
        if (errno != ENOENT) {  // Se o erro não for ENOENT, retorna erro
            so_error("S1.3", "");
            return RETURN_ERROR;
        } else {
            so_success("S1.3", "");
        }
    }
    
    // S1.4
    shmId = shmget(IPC_KEY, sizeof(DadosServidor), IPC_CREAT | IPC_EXCL | 0600);    // Cria a Shared Memory
    if (shmId == -1) {  // Se não conseguir criar a Shared Memory, retorna erro
        so_error("S1.4", "");
        return RETURN_ERROR;
    }
    database = (DadosServidor *) shmat(shmId, NULL, 0);     // Liga-se à Shared Memory
    if (database == (DadosServidor *) -1) {
        so_error("S1.4", "");
        return RETURN_ERROR;
    }
    so_success("S1.4", "%d", shmId);

    // S1.5
    for (int i = 0; i < MAX_PASSENGERS; i++) {  // Inicializa a lista de passageiros com valores nulos
        database->listClients[i].nif = PASSENGER_NOT_FOUND;
    }
    for (int i = 0; i < MAX_FLIGHTS; i++) { // Inicializa a lista de voos com valores nulos
        strcpy(database->listFlights[i].nrVoo, FLIGHT_NOT_FOUND);
    }
    so_success("S1.5", "");

    // S1.6
    CheckIn tempCI; // Variável temporária para guardar os dados dos passageiros
    FILE *file_passageiros = fopen("bd_passageiros.dat", "r");  // Abre o ficheiro dos passageiros
    if (file_passageiros == NULL) {
        so_error("S1.6", "");
        return RETURN_ERROR;
    }
    int i = 0;
    while (fread(&tempCI, sizeof(CheckIn), 1, file_passageiros) == 1) { // Lê os dados dos passageiros do ficheiro
        database->listClients[i].nif = tempCI.nif;
        strcpy(database->listClients[i].senha, tempCI.senha);
        strcpy(database->listClients[i].nrVoo, tempCI.nrVoo);
        strcpy(database->listClients[i].nome, tempCI.nome);
        database->listClients[i].lugarEscolhido = tempCI.lugarEscolhido;
        database->listClients[i].pidCliente = PID_INVALID;
        database->listClients[i].pidServidorDedicado = PID_INVALID;
        i++;
    }
    fclose(file_passageiros);
    so_success("S1.6", "");

    // S1.7
    Voo tempF;  // Variável temporária para guardar os dados dos voos
    FILE *file_voos = fopen("bd_voos.dat", "rb");   // Abre o ficheiro dos voos
    if (file_voos == NULL) {
        so_error("S1.7", "");
        return RETURN_ERROR;
    }
    i = 0;
    while (fread(&tempF, sizeof(Voo), 1, file_voos)){   // Lê os dados dos voos do ficheiro
        database->listFlights[i] = tempF;
        i++;
    }
    fclose(file_voos);
    so_success("S1.7", "");

    return shmId;
}

/**
 * @brief S2: Ler a descrição da tarefa no enunciado
 * @return o valor de msgId em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int initMsg_S2 () {
    msgId = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    msgId = msgget(IPC_KEY, 0);     // Verifica se a Message Queue já existe
    // S2.1
    if (msgId != -1) {
        if (msgctl(msgId, IPC_RMID, NULL) == -1) {  // Se existir, remove-a
            so_error("S2.1", "");
            return RETURN_ERROR;
        }
        so_success("S2.1", "");
    }

    // S2.2
    msgId = msgget(IPC_KEY, IPC_CREAT | IPC_EXCL | 0600);   // Cria a Message Queue
    if (msgId == -1) {
        so_error("S2.2", "");
        return RETURN_ERROR;
    }
    so_success("S2.2", "%d", msgId);

    so_debug("> [@return:%d]", msgId);
    return msgId;
}

/**
 * @brief S3: Ler a descrição da tarefa no enunciado
 * @return o valor de semId em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int initSem_S3 () {
    semId = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    int semVal[3] = {SEM_MUTEX_INITIAL_VALUE, SEM_MUTEX_INITIAL_VALUE, 0};  // Valores iniciais dos semáforos

    // S3.1
    semId = semget(IPC_KEY, 3, IPC_CREAT | IPC_EXCL | 0600);    // Cria o Grupo de Semáforos
    if (semId != RETURN_ERROR) {    // Se conseguir criar, remove-o
        if (semctl(semId, 0, IPC_RMID) == -1) { // Se não conseguir remover, retorna erro
            so_error("S3.1", "");
            return RETURN_ERROR;
        } else {
            so_success("S3.1", "");
        }
    }

    // S3.2
    semId = semget(IPC_KEY, 3, IPC_CREAT | IPC_EXCL | 0600);    // Cria o Grupo de Semáforos
    if (semId == RETURN_ERROR) {    // Se não conseguir criar, retorna erro
        so_error("S3.2", "");
        return RETURN_ERROR;
    }
    so_success("S3.2", "%d", semId);

    // S3.3
    if (semctl(semId, 0, SETVAL, semVal[0]) == -1) {    // Inicializa os semáforos
        so_error("S3.3", "");
        return RETURN_ERROR;
    }
    so_success("S3.3", "S");

    if (semctl(semId, 1, SETVAL, semVal[1]) == -1) {    // Inicializa os semáforos
        so_error("S3.3", "");
        return RETURN_ERROR;
    }
    so_success("S3.3", "");

    if (semctl(semId, 2, SETVAL, semVal[2]) == -1) {    // Inicializa os semáforos
        so_error("S3.3", "");
        return RETURN_ERROR;
    }
    so_success("S3.3", "");

    so_debug("> [@return:%d]", semId);
    return semId;
}

/**
 * @brief S4: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int triggerSignals_S4 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    if (signal(SIGINT, trataSinalSIGINT_S8) == SIG_ERR) {   // S4: "Se falhar a definição do sinal SIGINT, retorna erro"
        so_error("S4", "");
        return RETURN_ERROR;
    }

    if (signal(SIGCHLD, trataSinalSIGCHLD_S9) == SIG_ERR) { // S4: "Se falhar a definição do sinal SIGCHLD, retorna erro"
        so_error("S4", "");
        return RETURN_ERROR;
    }

    so_success("S4", "");
    return RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief S5: O CICLO1 já está a ser feito na função main(). Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int readRequest_S5 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    result = msgrcv(msgId, &clientRequest, sizeof(MsgContent) - sizeof(long), MSGTYPE_LOGIN, 0);    // Lê a mensagem da Message Queue
    if (result == -1) { // Se não conseguir ler a mensagem, retorna erro
        if (errno == EINTR) {   // Se o erro for EINTR, retorna CICLO1_CONTINUE
            return CICLO1_CONTINUE; // S5: "Se receber um sinal (...) retorna o valor CICLO1_CONTINUE"
        } else {
            so_error("S5", "");
            return RETURN_ERROR;
        }
    } else {
        so_success("S5", "%d %s %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.senha, clientRequest.msgData.infoCheckIn.pidCliente); // S5: "Se a leitura for bem-sucedida, imprime no ecrã os dados do pedido"
        return RETURN_SUCCESS;
    }

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief S6: Ler a descrição da tarefa no enunciado
 * @return PID do processo filho, se for o processo Servidor (pai),
 *         0 se for o processo Servidor Dedicado (filho),
 *         ou PID_INVALID (-1) em caso de erro
 */
int createServidorDedicado_S6 () {
    int pid_filho = PID_INVALID;    // Por omissão retorna erro
    so_debug("<");

    pid_filho = fork(); // Cria o processo filho
    if (pid_filho == -1) {  // Se não conseguir criar o processo filho, retorna erro
        so_error("S6", "");
        return -1;
    } else if (pid_filho == 0) {    // Se for o processo filho, retorna 0
        so_success("S6", "Servidor Dedicado: Nasci");
        return 0;
    } else {    // Se for o processo pai, retorna o PID do processo filho
        so_success("S6", "Servidor: Iniciei SD %d", pid_filho);
        nrServidoresDedicados++;
        return pid_filho;
    }

    so_debug("> [@return:%d]", pid_filho);
    return pid_filho;
}

/**
 * @brief S7: Ler a descrição da tarefa no enunciado
 */
void terminateServidor_S7 () {
    so_debug("<");

    so_success("S7", "Servidor: Start Shutdown");   // S7: "Imprime no ecrã a mensagem 'Servidor: Start Shutdown'"

    int saltar75 = 0;

    if (shmId != -1 && database != NULL) {  // S7.1
        so_success("S7.1", "");
    } else {
        so_error("S7.1", "");
        saltar75 = 1;
    }

    if (saltar75 == 0) {
        // S7.2
        for (int i = 0; i < MAX_PASSENGERS; i++) {      // S7.2: "Para cada cliente que tenha um Servidor Dedicado associado, envia um sinal SIGUSR2"
            if (database->listClients[i].pidServidorDedicado > 0) {
                kill(database->listClients[i].pidServidorDedicado, SIGUSR2);
                so_success("S7.2", "Servidor: Shutdown SD %d", database->listClients[i].pidServidorDedicado);
            }
        }

        // S7.3 
        struct sembuf S7;
        S7.sem_num = 2;
        S7.sem_op = -nrServidoresDedicados;
        S7.sem_flg = 0;

        if (semop(semId, &S7, 1) == -1) {  // S7.3: "Espera que todos os Servidores Dedicados terminem"
            so_error("S7.3", "");
        }else{
            so_success("S7.3", "");
        }

        // S7.4
        FILE *file_passageiros = fopen("bd_passageiros.dat", "w");   // S7.4: "Guarda os dados dos passageiros e dos voos nos ficheiros"
        if (file_passageiros == NULL) { 
            so_error("S7.4", "");
        }
        if (fwrite(database->listClients, sizeof(CheckIn), MAX_PASSENGERS, file_passageiros) != MAX_PASSENGERS) {
            so_error("S7.4", "");
        }
        fclose(file_passageiros);

        FILE *file_voos = fopen("bd_voos.dat", "w");    // S7.4: "Guarda os dados dos passageiros e dos voos nos ficheiros"
        if (file_voos == NULL) {
            so_error("S7.4", "");
        }
        if (fwrite(database->listFlights, sizeof(Voo), MAX_FLIGHTS, file_voos) != MAX_FLIGHTS) {
            so_error("S7.4", "");
        }
        fclose(file_voos);
        so_success("S7.4", "");
    }

    shmctl(shmId, IPC_RMID, NULL); //"Remove a Shared Memory
    semctl(semId, 0, IPC_RMID, 0);  //Remove o Grupo de Semáforos
    msgctl(msgId, IPC_RMID, NULL);  //Remove a Message Queue
    so_success("S7.5", "Servidor: End Shutdown");   //Imprime no ecrã a mensagem 'Servidor: End Shutdown'
    exit(0);

    so_debug(">");
}

/**
 * @brief S8: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_S8 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("S8", ""); 
    terminateServidor_S7(); // Chama a função terminateServidor_S7()

    so_debug(">");
}

/**
 * @brief S9: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGCHLD_S9 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    if (sinalRecebido == SIGCHLD) { // Se o sinal recebido for SIGCHLD
        int status;
        pid_t pid = pid;
        if ((pid = wait(&status)) > 0){ // Se o processo filho terminou
            if (WIFEXITED(status)){ // Se o processo filho terminou normalmente
                struct sembuf S9;
                S9.sem_num = 2;
                S9.sem_op = 1;
                S9.sem_flg = 0;

                semop(semId, &S9, 1); // Incrementa o semáforo 2
                so_success("S9", "Servidor: Confirmo fim de SD %d", pid);
                nrServidoresDedicados--;
            }
        }
    }

    so_debug(">");
}

/**
 * @brief SD10: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int triggerSignals_SD10 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    if (signal(SIGUSR1, trataSinalSIGUSR1_SD19) == SIG_ERR) { // Se falhar a definição do sinal SIGUSR1, retorna erro
        so_error("SD10", "");
        return RETURN_ERROR;
    }

    if (signal(SIGUSR2, trataSinalSIGUSR2_SD20) == SIG_ERR) {   // Se falhar a definição do sinal SIGUSR2, retorna erro
        so_error("SD10", "");
        return RETURN_ERROR;
    }

    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {   // Se falhar a definição do sinal SIGINT, retorna erro
        so_error("SD10", "");
        return RETURN_ERROR;
    }

    so_success("SD10", "");
    return RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD11: Ler a descrição da tarefa no enunciado
 * @return indexClient em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int searchClientDB_SD11 () {
    indexClient = -1;    // SD11: Inicia a variável indexClient a -1 (índice da lista de passageiros de database)
    so_debug("<");


    // SD11.1
    int i;
    for (i = 0; i < MAX_PASSENGERS; i++) { // Procura o cliente na BD
        if (database->listClients[i].nif == clientRequest.msgData.infoCheckIn.nif) {
            indexClient = i;
            break;
        }
    }

    if (indexClient == -1) {   // Se não encontrar o cliente, retorna erro
        so_error("SD11.1", "Cliente %d: não encontrado", clientRequest.msgData.infoCheckIn.nif);
        return RETURN_ERROR;
    }

    if (database->listClients[indexClient].nif != clientRequest.msgData.infoCheckIn.nif) { // Se o NIF do cliente não for igual ao NIF do pedido, retorna erro
        searchClientDB_SD11();
    }

    if (strcmp(database->listClients[indexClient].senha, clientRequest.msgData.infoCheckIn.senha) != 0) { // Se a senha do cliente não for igual à senha do pedido, retorna erro
        so_error("SD11.3", "Cliente %d: Senha errada", clientRequest.msgData.infoCheckIn.nif);
        return RETURN_ERROR;
    }
    so_success("SD11.3", "%d", indexClient);

    so_debug("> [@return:%d]", indexClient);
    return indexClient;
}

/**
 * @brief SD12: Ler a descrição da tarefa no enunciado
 * @return indexFlight em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int searchFlightDB_SD12 () {
    indexFlight = -1;    // SD12: Inicia a variável indexFlight a -1 (índice da lista de voos de database)
    so_debug("<");

    for (int i = 0; i < MAX_FLIGHTS; i++) {
        if (strcmp(database->listFlights[i].nrVoo, database->listClients[indexClient].nrVoo) == 0) { // Procura o voo na BD
            indexFlight = i;
            so_success("SD12.2", "%d", indexFlight);
            return indexFlight;
        }
    }

    if (indexFlight == -1) {   // Se não encontrar o voo, retorna erro
        so_error("SD12.1", "Voo %s: não encontrado", database->listClients[indexClient].nrVoo); 
    }

    so_debug("> [@return:%d]", indexFlight);
    return indexFlight;
}

/**
 * @brief SD13: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int updateClientDB_SD13 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    so_success("SD13.1", "Start Check-in: %d %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.pidCliente);

    struct sembuf downSD13, upSD13; // Inicializa os semáforos
    downSD13.sem_num = 0;
    downSD13.sem_op = -1;
    downSD13.sem_flg = 0;

    upSD13.sem_num = 0;
    upSD13.sem_op = 1;
    upSD13.sem_flg = 0;

    if (semop(semId, &downSD13, 1) == -1) { // Adquire o semáforo
        so_error("SD13", "Semaphore wait error");
        return RETURN_ERROR;
    }
 
    if (database->listClients[indexClient].pidCliente != PID_INVALID || database->listClients[indexClient].lugarEscolhido != EMPTY_SEAT) { // Se o cliente já fez check-in, retorna erro
        so_error("SD13.2", "Cliente %d: Já fez check-in", clientRequest.msgData.infoCheckIn.nif);
        // Release semaphore before returning error
        if (semop(semId, &upSD13, 1) == -1) {
            so_error("SD13", "Semaphore signal error");
        }
        return RETURN_ERROR;
    }

    sleep(4);  // Simula o tempo de processamento
 
    database->listClients[indexClient].pidCliente = clientRequest.msgData.infoCheckIn.pidCliente; // Atualiza a BD
    database->listClients[indexClient].pidServidorDedicado = getpid();

    if (semop(semId, &upSD13, 1) == -1) { // Liberta o semáforo
        so_error("SD13", "Semaphore signal error");
        return RETURN_ERROR;
    }
    
    so_success("SD13.5", "End Check-in: %d %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.pidCliente);
    return RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD14: Ler a descrição da tarefa no enunciado
 * @param erroValidacoes booleano que diz se houve algum erro nas validações de SD11, SD12 e SD13
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int sendResponseClient_SD14 (int erroValidacoes) {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("< [@param erroValidacoes:%d]", erroValidacoes);

    if (erroValidacoes) {  // Se houve erro nas validações, envia mensagem de erro 
        so_error("SD14.1", ""); 
        clientRequest.msgData.infoCheckIn.lugarEscolhido = EMPTY_SEAT;    // Envia lugarEscolhido a EMPTY_SEAT
        clientRequest.msgData.infoCheckIn.pidServidorDedicado = PID_INVALID; // Envia pidServidorDedicado a PID_INVALID
    } else {
        so_success("SD14.1", "");
        clientRequest.msgData.infoCheckIn.pidServidorDedicado = getpid();
        clientRequest.msgData.infoCheckIn.lugarEscolhido = EMPTY_SEAT;
        clientRequest.msgData.infoVoo = database->listFlights[indexFlight];
        clientRequest.msgType = clientRequest.msgData.infoCheckIn.pidCliente;
    }

    if (msgsnd(msgId, &clientRequest, sizeof(MsgContent)-sizeof(long), 0) == -1) {
        so_error("SD14.2", "");
        return RETURN_ERROR;
    } else {
        so_success("SD14.2", "");
        return RETURN_SUCCESS;
    }

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD15: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int readResponseClient_SD15 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    result = msgrcv(msgId, &clientRequest, sizeof(MsgContent) - sizeof(long), getpid(), 0);   // Lê a mensagem da Message Queue
    if (result == -1) { // Se não conseguir ler a mensagem, retorna erro
        so_error("SD15", "");
        return RETURN_ERROR;
    } else {
        so_success("SD15", "%d %d %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.lugarEscolhido, clientRequest.msgData.infoCheckIn.pidCliente);
        return RETURN_SUCCESS;
    }

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD16: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int updateFlightDB_SD16 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    so_success("SD16.1", "Start Reserva lugar: %s %d %d", clientRequest.msgData.infoCheckIn.nrVoo, clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.lugarEscolhido);

    struct sembuf downSD16, upSD16; // Inicializa os semáforos
    downSD16.sem_num = 1;
    downSD16.sem_op = -1;
    downSD16.sem_flg = 0;

    upSD16.sem_num = 1;
    upSD16.sem_op = 1;
    upSD16.sem_flg = 0;

    // Acquire semaphore
    if (semop(semId, &downSD16, 1) == -1) { // Adquire o semáforo
        so_error("SD16", "");
        return RETURN_ERROR;
    }

    if (database->listFlights[indexFlight].lugares[clientRequest.msgData.infoCheckIn.lugarEscolhido] != EMPTY_SEAT) {    // Se o lugar escolhido já estiver ocupado, retorna erro
        so_error("SD16.2", "Cliente %d: Lugar já estava ocupado", clientRequest.msgData.infoCheckIn.nif);
        if (semop(semId, &upSD16, 1) == -1) {
            so_error("SD16", "");
        }
        return RETURN_ERROR;
    }

    sleep(4);   // Simula o tempo de processamento

    database->listFlights[indexFlight].lugares[clientRequest.msgData.infoCheckIn.lugarEscolhido] = clientRequest.msgData.infoCheckIn.nif; // Atualiza a BD
    database->listClients[indexClient].lugarEscolhido = clientRequest.msgData.infoCheckIn.lugarEscolhido;   // Atualiza a BD

    if (semop(semId, &upSD16, 1) == -1) {   // Liberta o semáforo
        so_error("SD16", "");
        return RETURN_ERROR;
    }  

    so_success("SD16.6", "End Reserva lugar: %s %d %d", clientRequest.msgData.infoCheckIn.nrVoo, clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.lugarEscolhido);  // Imprime no ecrã a mensagem 'End Reserva lugar: %s %d %d'
    return RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD17: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int sendConfirmationClient_SD17 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    clientRequest.msgType = clientRequest.msgData.infoCheckIn.pidCliente; // Envia a mensagem para o cliente
    clientRequest.msgData.infoCheckIn.pidServidorDedicado = getpid(); // Envia o PID do Servidor Dedicado
    database->listClients[indexClient].lugarEscolhido = clientRequest.msgData.infoCheckIn.lugarEscolhido;
    strcpy(database->listFlights[indexClient].origem, clientRequest.msgData.infoVoo.origem);
    strcpy(database->listFlights[indexClient].destino, clientRequest.msgData.infoVoo.destino);

    if (msgsnd(msgId, &clientRequest, sizeof(MsgContent) - sizeof(long), 0) == -1) {    // Envia a mensagem para a Message Queue
        so_error("SD17", "");
        return RETURN_ERROR;
    } else {
        so_success("SD17", "");
        return RETURN_SUCCESS;
    }

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD18: Ler a descrição da tarefa no enunciado
 */
void terminateServidorDedicado_SD18 () {
    so_debug("<");

    if (indexClient >= 0) { // Se o índice do cliente for válido, atualiza a BD
        database->listClients[indexClient].pidCliente = PID_INVALID; 
        database->listClients[indexClient].pidServidorDedicado = PID_INVALID;
        so_success("SD18", "");
    }

    so_debug(">");
    exit(0);
}

/**
 * @brief SD19: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR1_SD19 (int sinalRecebido) { 
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("SD19", "SD: Recebi pedido do Cliente para terminar"); // Imprime no ecrã a mensagem 'SD: Recebi pedido do Cliente para terminar'
    terminateServidorDedicado_SD18();

    so_debug(">");
}

/**
 * @brief SD20: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR2_SD20 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("SD20", "SD: Recebi pedido do Servidor para terminar");

    if (clientRequest.msgData.infoCheckIn.pidCliente != PID_INVALID) { // Se o PID do cliente for válido, envia um sinal SIGHUP
        kill(clientRequest.msgData.infoCheckIn.pidCliente, SIGHUP); 
    }

    terminateServidorDedicado_SD18(); // Termina o Servidor Dedicado

    so_debug(">");
}