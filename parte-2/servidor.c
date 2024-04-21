/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2023/2024, Enunciado Versão 3+
 **
 ** Aluno: Nº: 105054  Nome: Hugo Magalhães do Vale Rosa Serra
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo:
 **
 ** Este script implementa um servidor para um sistema de check-in online de uma companhia 
 ** aérea. O servidor é composto por um processo principal (pai) e vários processos dedicados 
 ** (filhos). Ele lê pedidos de clientes de um FIFO, verifica a base de dados dos passageiros, realiza o 
 ** check-in, envia uma confirmação de sucesso ou falha de check-in de volta ao cliente e atualiza a 
 ** base de dados. O servidor trata sinais para interrupção, término de processos dedicados e shutdown.
 ******************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/*** Variáveis Globais ***/
CheckIn clientRequest; // Variável que tem o pedido enviado do Cliente para o Servidor

/**
 * @brief Processamento do processo Servidor e dos processos Servidor Dedicado
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main () {
    // S1
    checkExistsDB_S1(FILE_DATABASE);
    // S2
    createFifo_S2(FILE_REQUESTS);
    // S3
    triggerSignals_S3(FILE_REQUESTS);

    int indexClient;       // Índice do cliente que fez o pedido ao servidor/servidor dedicado na BD

    // S4: CICLO1
    while (TRUE) {
        // S4
        clientRequest = readRequest_S4(FILE_REQUESTS); // S4: "Se houver erro (...) clientRequest.nif == -1"
        if (clientRequest.nif < 0)   // S4: "Se houver erro na abertura do FIFO ou na leitura do mesmo, (...)"
            continue;                // S4: "(...) e recomeça o Ciclo1 neste mesmo passo S4, lendo um novo pedido"

        // S5
        int pidServidorDedicado = createServidorDedicado_S5();
        if (pidServidorDedicado > 0) // S5: "o processo Servidor (pai) (...)"
            continue;                // S5: "(...) recomeça o Ciclo1 no passo S4 (ou seja, volta a aguardar novo pedido)"
        // S5: "o Servidor Dedicado (que tem o PID pidServidorDedicado) segue para o passo SD9"

        // SD9
        triggerSignals_SD9();
        // SD10
        CheckIn itemBD;
        indexClient = searchClientDB_SD10(clientRequest, FILE_DATABASE, &itemBD);
        // SD11
        checkinClientDB_SD11(&clientRequest, FILE_DATABASE, indexClient, itemBD);
        // SD12
        sendAckCheckIn_SD12(clientRequest.pidCliente);
        // SD13
        closeSessionDB_SD13(clientRequest, FILE_DATABASE, indexClient);
        so_exit_on_error(-1, "ERRO: O servidor dedicado nunca devia chegar a este ponto");
    }
}

/**
 *  "O módulo Servidor é responsável pelo processamento do check-in dos passageiros. 
 *   Está dividido em duas partes, um Servidor (pai) e zero ou mais Servidores Dedicados (filhos).
 *   Este módulo realiza as seguintes tarefas:"
 */

/**
 * @brief S1     Ler a descrição da tarefa S1 no enunciado
 * @param nameDB O nome da base de dados (i.e., FILE_DATABASE)
 */
void checkExistsDB_S1 (char *nameDB) {
    so_debug("< [@param nameDB:%s]", nameDB);

    if (access(nameDB, R_OK | W_OK) != -1){  // Verificar se a base de dados existe e é acessível para leitura e escrita
        so_success("S1", "");
    }else{
        so_error("S1", "");
        exit(1);
    }

    so_debug(">");
}

/**
 * @brief S2       Ler a descrição da tarefa S2 no enunciado
 * @param nameFifo O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void createFifo_S2 (char *nameFifo) {
    so_debug("< [@param nameFifo:%s]", nameFifo);

    if(access(nameFifo, F_OK) != -1){    // Verificar se o FIFO já existe
        unlink(nameFifo);          // Se existir, apagar o FIFO
    }
    
    if (mkfifo(nameFifo, 0666) == -1) { // Criar o FIFO do servidor
        so_error("S2", "");
        exit(1);
    }else{
        so_success("S2", "");
    }

    so_debug(">");
}

/**
 * @brief S3   Ler a descrição da tarefa S3 no enunciado
 */
void triggerSignals_S3 () {
    so_debug("<");

    if (signal(SIGINT, trataSinalSIGINT_S6) == SIG_ERR){ // Arma e trata o sinal SIGINT
        so_error("S3", "");
        so_debug(">");
        deleteFifoAndExit_S7();
    }

    if (signal(SIGCHLD, trataSinalSIGCHLD_S8) == SIG_ERR){ // Arma e trata o sinal SIGCHLD
        so_error("S3", "");
        so_debug(">");
        deleteFifoAndExit_S7();
    }

    so_success("S3", "");

    so_debug(">");
}

/**
 * @brief S4       O CICLO1 já está a ser feito na função main(). Ler a descrição da tarefa S4 no enunciado
 * @param nameFifo O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 * @return CheckIn Elemento com os dados preenchidos. Se nif=-1, significa que o elemento é inválido
 */
CheckIn readRequest_S4 (char *nameFifo) {
    CheckIn request;
    request.nif = -1;   // Por omissão retorna erro
    so_debug("< [@param nameFifo:%s]", nameFifo);

    FILE *fd = fopen(nameFifo, "r");    // Abrir o FIFO do servidor para leitura 
    if (fd == NULL){    // Verificar se o FIFO foi aberto com sucesso
        so_error("S4", "");
        deleteFifoAndExit_S7();
    }

    int result = fscanf(fd, "%d %s %d", &request.nif, request.senha, &request.pidCliente);  // Ler as informações do cliente
    fclose(fd);   // Fechar o FIFO

    if (result != 3 || request.nif <= 0 || request.nif > 999999999 || request.pidCliente <=0 || strlen(request.senha) == 0) {   // Verificar se as informações do cliente são válidas
        so_error("S4", "");
        deleteFifoAndExit_S7();
    }
    so_success("S4", "%d %s %d", request.nif, request.senha, request.pidCliente); 

    so_debug("> [@return nif:%d, senha:%s, pidCliente:%d]", request.nif, request.senha, request.pidCliente);
    return request;
}

/**
 * @brief S5   Ler a descrição da tarefa S5 no enunciado
 * @return int PID do processo filho, se for o processo Servidor (pai),
 *             0 se for o processo Servidor Dedicado (filho), ou -1 em caso de erro.
 */
int createServidorDedicado_S5 () {
    int pid_filho = -1;    // Por omissão retorna erro
    so_debug("<");

    pid_filho = fork(); // Criar um processo filho (fork)

    if (pid_filho == -1){   // Verificar se o processo filho foi criado com sucesso
        so_error("S5", "");
        deleteFifoAndExit_S7();
    }

    if (pid_filho == 0){
        triggerSignals_SD9();
    }

    so_success("S5", "Servidor: Iniciei SD %d", pid_filho);

    so_debug("> [@return:%d]", pid_filho);
    return pid_filho;
}

/**
 * @brief S6            Ler a descrição das tarefas S6 e S7 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_S6 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    if (sinalRecebido == SIGINT){   // Verificar se o sinal recebido é SIGINT
        so_success("S6", "Servidor: Start Shutdown");
    }

    FILE *dbFile = fopen("bd_passageiros.dat", "r"); // Abre o ficheiro bd_passageiros.dat para leitura.
    if (dbFile == NULL) {   // Verificar se o ficheiro foi aberto com sucesso
        so_error("S6.1", "");
        deleteFifoAndExit_S7();
    }
    so_success("S6.1", "");

    CheckIn itemDB;

    while (TRUE){  //CICLO 2
        if (fread(&itemDB, sizeof(CheckIn), 1, dbFile) != 1) { // Ler os dados do ficheiro bd_passageiros.dat
            if (feof(dbFile)) { // Verificar se o ficheiro chegou ao fim
                so_success("S6.2", "");
                deleteFifoAndExit_S7(); 
            } else {
                so_error("S6.2", "");
                deleteFifoAndExit_S7();
            }
        }

        if (itemDB.pidServidorDedicado > 0) {   // Verificar se o PID do Servidor Dedicado é válido
            kill(itemDB.pidServidorDedicado, SIGUSR2);  // Enviar sinal SIGUSR2 para o Servidor Dedicado
            so_success("S6.3", "Servidor: Shutdown SD %d", itemDB.pidServidorDedicado);
        }
    }

    fclose(dbFile); // Fechar o ficheiro bd_passageiros.dat

    deleteFifoAndExit_S7(); // Apagar o FIFO e terminar o processo

    so_debug(">");
}

/**
 * @brief S7 Ler a descrição da tarefa S7 no enunciado
 */
void deleteFifoAndExit_S7 () {
    so_debug("<");

    if (unlink("server.fifo") == 0){    // Apagar o FIFO do servidor
        so_success("S7", "Servidor: End Shutdown");
    }else{
        so_error("S7", "");
    }

    so_debug(">");
    exit(0);
}

/**
 * @brief S8            Ler a descrição da tarefa S8 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGCHLD_S8 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    if (sinalRecebido == SIGCHLD){  // Verificar se o sinal recebido é SIGCHLD
        int status;
        pid_t pid = wait(&status);
        so_success("S8", "Servidor: Confirmo fim de SD %d", pid);
    }

    so_debug(">");
}

/**
 * @brief SD9  Ler a descrição da tarefa SD9 no enunciado
 */
void triggerSignals_SD9 () {
    so_debug("<");

    if (signal(SIGUSR2, trataSinalSIGUSR2_SD14) == SIG_ERR) {   // Arma e trata o sinal SIGUSR2
        so_error("SD9", "");
        exit(1);
    }

    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {   // Arma e trata o sinal SIGINT
        so_error("SD9", "");
        exit(1);
    }

    so_debug(">");
}

/**
 * @brief SD10    Ler a descrição da tarefa SD10 no enunciado
 * @param request O pedido do cliente
 * @param nameDB  O nome da base de dados
 * @param itemDB  O endereço de estrutura CheckIn a ser preenchida nesta função com o elemento da BD
 * @return int    Em caso de sucesso, retorna o índice de itemDB no ficheiro nameDB.
 */
int searchClientDB_SD10 (CheckIn request, char *nameDB, CheckIn *itemDB) {
    int indexClient = 0;
    so_debug("< [@param request.nif:%d, request.senha:%s, nameDB:%s, itemDB:%p]", request.nif, request.senha, nameDB, itemDB);

    FILE *dbFile = fopen(nameDB, "r");  // Abrir o ficheiro bd_passageiros.dat para leitura
    if (dbFile == NULL) {   // Verificar se o ficheiro foi aberto com sucesso   
        so_error("SD10", "");
        exit(1);
    }

    while (TRUE){   // CICLO 3
        if (fread(itemDB, sizeof(CheckIn), 1, dbFile) != 1) {   // Ler os dados do ficheiro bd_passageiros.dat
            if (feof(dbFile)) { // Verificar se o ficheiro chegou ao fim
                so_error("SD10.1", "Cliente %d: não encontrado", request.nif);
                kill(request.pidCliente, SIGHUP);
                exit(1);
            } else {
                so_error("SD10.1", "");
                exit(1);
            }
        }

        if (itemDB->nif == request.nif) {   // Verificar se o NIF do cliente é válido
            if (strcmp(itemDB->senha, request.senha) == 0) {    // Verificar se a senha do cliente é válida
                so_success("SD10.3", "%d", indexClient);
                break;
            } else {
                so_error("SD10.3", "Cliente %d: Senha errada", request.nif);
                kill(request.pidCliente, SIGHUP);
                exit(1);
            }
        }

        indexClient++;
    }

    fclose(dbFile); // Fechar o ficheiro bd_passageiros.dat

    so_success("SD10", "");
    
    so_debug("> [@return:%d, nome:%s, nrVoo:%s]", indexClient, itemDB->nome, itemDB->nrVoo);
    return indexClient;
}

/**
 * @brief SD11        Ler a descrição da tarefa SD11 no enunciado
 * @param request     O endereço do pedido do cliente (endereço é necessário pois será alterado)
 * @param nameDB      O nome da base de dados
 * @param indexClient O índica na base de dados do elemento correspondente ao cliente
 * @param itemDB      O elemento da BD correspondente ao cliente
 */
void checkinClientDB_SD11 (CheckIn *request, char *nameDB, int indexClient, CheckIn itemDB) {
    so_debug("< [@param request:%p, nameDB:%s, indexClient:%d, itemDB.pidServidorDedicado:%d]",
                                    request, nameDB, indexClient, itemDB.pidServidorDedicado);

    strcpy(request->nome, itemDB.nome); // Preenche o nome com os valores do itemDB 
    strcpy(request->nrVoo, itemDB.nrVoo);   // Preenche o nrVoo com os valores do itemDB
    request->pidServidorDedicado = getpid();    // Preenche o pidServidorDedicado com o PID do Servidor Dedicado
    so_success("SD11.1", "%s %s %d", request->nome, request->nrVoo, request->pidServidorDedicado);

    FILE *file = fopen("bd_passageiros.dat", "r+"); // Abrir o ficheiro bd_passageiros.dat para leitura e escrita
    if (file == NULL) { // Verificar se o ficheiro foi aberto com sucesso
        so_error("SD11.2", "");
        kill(request->pidCliente, SIGHUP);
        exit(1);
    }
    so_success("SD11.2", "");

    int seek = fseek(file, indexClient * sizeof(CheckIn), SEEK_SET);    // Posicionar o ficheiro para o início do elemento itemDB correspondente ao cliente
    if (seek == -1) {   // Verificar se o ficheiro foi posicionado com sucesso
        so_error("SD11.3", "");
        kill(request->pidCliente, SIGHUP);
        exit(1);
    }
    so_success("SD11.3", "");

    int writeResult = fwrite(request, sizeof(CheckIn), 1, file);    // Escrever os dados do cliente no ficheiro bd_passageiros.dat
    int closeResult = fclose(file); // Fechar o ficheiro bd_passageiros.dat
    if (writeResult != 1 || closeResult != 0) {  // Verificar se os dados foram escritos com sucesso e se o ficheiro foi fechado com sucesso
        so_error("SD11.4", "");
        kill(request->pidCliente, SIGHUP);
        exit(1);
    }
    so_success("SD11.4", "");


    so_debug("> [nome:%s, nrVoo:%s, pidServidorDedicado:%d]", request->nome,
                                                request->nrVoo, request->pidServidorDedicado);
}

/**
 * @brief SD12       Ler a descrição da tarefa SD12 no enunciado
 * @param pidCliente PID (Process ID) do processo Cliente
 */
void sendAckCheckIn_SD12 (int pidCliente) {
    so_debug("< [@param pidCliente:%d]", pidCliente);

    srand(time(NULL));  // Inicializa o gerador de números aleatórios

    int waitTime = rand() % MAX_ESPERA + 1; // Tempo de espera aleatório entre 1 e MAX_ESPERA

    so_success("SD12","%d", waitTime);

    sleep(waitTime);    // Esperar waitTime segundos

    kill(pidCliente, SIGUSR1);  // Enviar sinal SIGUSR1 para o Cliente

    so_debug(">");
}

/**
 * @brief SD13          Ler a descrição da tarefa SD13 no enunciado
 * @param clientRequest O endereço do pedido do cliente
 * @param nameDB        O nome da base de dados
 * @param indexClient   O índica na base de dados do elemento correspondente ao cliente
 */
void closeSessionDB_SD13 (CheckIn clientRequest, char *nameDB, int indexClient) {
    so_debug("< [@param clientRequest:%p, nameDB:%s, indexClient:%d]", &clientRequest, nameDB,
                                                                                    indexClient);

    clientRequest.pidCliente = -1;  // Preenche o pidCliente com -1
    clientRequest.pidServidorDedicado = -1;   // Preenche o pidServidorDedicado com -1

    FILE *file = fopen("bd_passageiros.dat", "r+"); // Abrir o ficheiro bd_passageiros.dat para leitura e escrita
    if (file == NULL) { // Verificar se o ficheiro foi aberto com sucesso
        so_error("SD13.1", "");
        exit(1);
    }
    so_success("SD13.1", "");

    int seek = fseek(file, indexClient * sizeof(CheckIn), SEEK_SET);
    if (seek == -1) {   // Verificar se o ficheiro foi posicionado com sucesso
        so_error("SD13.2", "");
        exit(1);
    }
    so_success("SD13.2", "");

    int writeResult = fwrite(&clientRequest, sizeof(CheckIn), 1, file); // Escrever os dados do cliente
    int closeResult = fclose(file); // Fechar o ficheiro
    if (writeResult != 1 || closeResult != 0) { // Verificar se os dados foram escritos com sucesso e se o ficheiro foi fechado com sucesso
        so_error("SD13.3", "");
        exit(1);
    }
    so_success("SD13.3", "");

    so_debug("> [pidCliente:%d, pidServidorDedicado:%d]", clientRequest.pidCliente, 
                                                          clientRequest.pidServidorDedicado);
}

/**
 * @brief SD14          Ler a descrição da tarefa SD14 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR2_SD14 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("SD14", "SD: Recebi pedido do Servidor para terminar");  // Recebeu pedido do Servidor para terminar
    exit(0);    // Termina o processo

    so_debug(">");
}