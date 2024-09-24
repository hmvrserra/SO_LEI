/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos 2023/2024, Enunciado Versão 1+
 **
 ** Este Módulo não deverá ser alterado, e não precisa ser entregue
 ** Nome do Módulo: common.h
 ** Descrição/Explicação do Módulo:
 **     Definição das estruturas de dados comuns aos módulos servidor e cliente
 **
 ******************************************************************************/
#ifndef __COMMON_H__
    #define __COMMON_H__

    #include "/home/so/utils/include/so_utils.h"
    #include <signal.h>     // Header para as constantes SIG_* e as funções signal() e kill()
    #include <unistd.h>     // Header para as funções alarm(), pause(), sleep(), fork(), exec*() e get*pid()
    #include <sys/ipc.h>    // Header para as funções de IPC
    #include <sys/sem.h>    // Header para as funções de IPC
    #include <sys/msg.h>    // Header para as funções de IPC
    #include <sys/shm.h>    // Header para as funções de IPC
    #include <sys/wait.h>   // Header para a função wait()
    #include <sys/stat.h>   // Header para as constantes S_ISFIFO e as funções stat() e mkfifo()

    #define FILE_DATABASE_PASSAGEIROS   "bd_passageiros.dat"    // Ficheiro de acesso direto que armazena a lista de passageiros
    #define FILE_DATABASE_VOOS          "bd_voos.dat"           // Ficheiro de acesso direto que armazena a lista de voos

    #define RETURN_SUCCESS       0      // Defines utilitários para valores de retorno
    #define RETURN_ERROR        -1      // Defines utilitários para valores de retorno
    #define CICLO1_CONTINUE      2      // Valor de retorno que indica que o main() deve recomeçar o CICLO1
    #define CYCLE1_CONTINUE      CICLO1_CONTINUE

    #define MAX_PASSENGERS      20      // Nº máximo de passageiros
    #define MAX_FLIGHTS         10      // Nº máximo de voos
    #define MAX_SEATS           11      // Nº máximo de lugares em cada voo (assume-se que os aviões são todos iguais)
    #define EMPTY_SEAT          -1      // Indica que uma entrada do array lugares está disponível
    #define PASSENGER_NOT_FOUND -1      // Valor de NIF indicando que o passageiro é inválido
    #define FLIGHT_NOT_FOUND    "NONE"  // Valor de nrVoo indicando que o voo é inválido
    #define PID_INVALID         -1      // Valor de PID inválido

    #define MAX_ESPERA           5      // Tempo máximo de espera por parte do Cliente
    #define MSGTYPE_LOGIN        1      // MsgType do Servidor Principal
    #define SEM_PASSAGEIROS      0      // Índice do semáforo no Grupo de Semáforos, relativo ao MUTEX da lista de Passageiros
    #define SEM_VOOS             1      // Índice do semáforo no Grupo de Semáforos, relativo ao MUTEX da lista de Voos
    #define SEM_NR_SRV_DEDICADOS 2      // Índice do semáforo no Grupo de Semáforos, relativo à espera do fim dos Servidores Dedicados
    #define SEM_MUTEX_INITIAL_VALUE 1   // Valor inicial dos semáforos do tipo MUTEX

    typedef struct {
        int  nif;                       // Número de contribuinte do passageiro
        char senha[20];                 // Senha do passageiro
        char nome[56];                  // Nome do passageiro
        char nrVoo[8];                  // Número do voo
        int  pidCliente;                // PID do processo Cliente
        int  pidServidorDedicado;       // PID do processo Servidor Dedicado
        int  lugarEscolhido;            // Indicação de qual foi o lugar escolhido
    } CheckIn;

    typedef struct {
        char nrVoo[8];                  // Número do voo
        char origem[24];                // Origem do voo
        char destino[24];               // Destino do voo
        int  lugares[MAX_SEATS];        // Lista de lugares disponíveis e ocupados do voo
    } Voo;

    typedef struct {
        long msgType;                   // Tipo da Mensagem
        struct {
            CheckIn infoCheckIn;        // Informação sobre o CheckIn
            Voo infoVoo;                // Informação sobre um Voo
        } msgData;                      // Dados da Mensagem
    } MsgContent;

    typedef struct {                    // Base de dados de Negócio, em Memória Partilhada
        CheckIn listClients[MAX_PASSENGERS]; // Lista de passageiros
        Voo listFlights[MAX_FLIGHTS];        // Lista de voos dos passageiros
    } DadosServidor;

    /* Protótipos de funções */
    int  initShm_S1 ();                         // S1:   Função a ser implementada pelos alunos
    int  initMsg_S2 ();                         // S2:   Função a ser implementada pelos alunos
    int  initSem_S3 ();                         // S3:   Função a ser implementada pelos alunos
    int  triggerSignals_S4 ();                  // S4:   Função a ser implementada pelos alunos
    int  readRequest_S5 ();                     // S5:   Função a ser implementada pelos alunos
    int  createServidorDedicado_S6 ();          // S6:   Função a ser implementada pelos alunos
    void terminateServidor_S7 ();               // S7:   Função a ser implementada pelos alunos
    void trataSinalSIGINT_S8 (int);             // S8:   Função a ser implementada pelos alunos
    void trataSinalSIGCHLD_S9 (int);            // S9:   Função a ser implementada pelos alunos
    int  triggerSignals_SD10 ();                // SD10: Função a ser implementada pelos alunos
    int  searchClientDB_SD11 ();                // SD11: Função a ser implementada pelos alunos
    int  searchFlightDB_SD12 ();                // SD12: Função a ser implementada pelos alunos
    int  updateClientDB_SD13 ();                // SD13: Função a ser implementada pelos alunos
    int  sendResponseClient_SD14 (int);         // SD14: Função a ser implementada pelos alunos
    int  readResponseClient_SD15 ();            // SD15: Função a ser implementada pelos alunos
    int  updateFlightDB_SD16 ();                // SD16: Função a ser implementada pelos alunos
    int  sendConfirmationClient_SD17 ();        // SD17: Função a ser implementada pelos alunos
    void terminateServidorDedicado_SD18();      // SD18: Função a ser implementada pelos alunos
    void trataSinalSIGUSR1_SD19 (int);          // SD19: Função a ser implementada pelos alunos
    void trataSinalSIGUSR2_SD20 (int);          // SD20: Função a ser implementada pelos alunos

    int  initMsg_C1 ();                         // C1:   Função a ser implementada pelos alunos
    int  triggerSignals_C2 ();                  // C2:   Função a ser implementada pelos alunos
    int  getDadosPedidoUtilizador_C3 ();        // C3:   Função a ser implementada pelos alunos
    int  sendRequest_C4 ();                     // C4:   Função a ser implementada pelos alunos
    void configureTimer_C5 (int);               // C5:   Função a ser implementada pelos alunos
    int  readResponseSD_C6 ();                  // C6:   Função a ser implementada pelos alunos
    int  trataResponseSD_C7 ();                 // C7:   Função a ser implementada pelos alunos
    int  sendSeatChoice_C8 (int);               // C8:   Função a ser implementada pelos alunos
    void terminateCliente_C9 ();                // C9:   Função a ser implementada pelos alunos
    void trataSinalSIGHUP_C10 (int);            // C10:  Função a ser implementada pelos alunos
    void trataSinalSIGINT_C11 (int);            // C11:  Função a ser implementada pelos alunos
    void trataSinalSIGALRM_C12 (int);           // C12:  Função a ser implementada pelos alunos
#endif  // __COMMON_H__