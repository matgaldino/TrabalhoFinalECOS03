/*
ECOS13 - Projeto Final - Prof. Dr. Otávio Gomes
SHORTEST REMAINING TIME NEXT - SRTN
GUARANTEED
LOTTERY
FAIR-SHARE
Daniel Ferreira Lara && Dielson Soares de Oliveira Junior && Matheus Siston Galdino
2021003661              2021004346                           2021006340
*/

#include<stdio.h>
#include<stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define BUFFERSIZE 11 //NUMERO DE MAXIMO DE PROCESSOS NO BUFFER = BUFFERSIZE - 1
#define MAX_PROCESSES 20 //MAXIMO DE PROCESSOS NO ARQUIVO
#define CPU_TIME 10
#define TIME_PER_PROCESS 2
#define EXEC 1
#define PRONTO 0
#define NOVO -1
#define FIM 2

typedef void (*ptrFunc)(void);

void criarProcessos(void);
void imprimeLinha(void);
void imprimeCabecalho(void);
bool addProc(int id);
void endProc(void);
void inicializaListaProcessos(void);
void criaProcesso(void);
void execute_next_process(void);
void clockTick(void);
void srtn(void);
void guaranteed(void);
void lottery(void);
void fairShare(void);


typedef struct{ //ESTRUTURA DOS PROCESSOS
   int identificador;
   int dataCriacao;
   int duracao;
   int tempoCorrido;
   int prioridade;
   char estado;
}Processo;

typedef struct{ //ESTRUTURA DO KERNEL
   ptrFunc scheduler;
}Kernel;

Processo buffer[BUFFERSIZE]; //CRIACAO DO BUFFER CIRCULAR
Processo lista[MAX_PROCESSES]; //LISTA COM TODOS OS PROCESSOS
Kernel kernels[4]; //CRIACAO DO KERNEL
int QUANTUM = 1; //DEFINICAO DO QUANTUM PADRAO
int maptolist[MAX_PROCESSES+1]; //FUNCAO AUXILIAR PARA IMPRESSAO
int t = -1; //VARIAVEL DE TEMPO
int startFila = 0; //VARIAVEL AUXILIAR PARA CAMINHAR NA LISTA DE PROCESSOS
int start = 0, end = 0; //VARIAVEIS AUXILIARES PARA CAMINHAR NO BUFFER CIRCULAR
int processosAtivos = 0; //VARIAVEL AUXILIAR PARA O ESCALONADOR FAIR-SHARE

/* CABECALHOS PARA CADA UM DOS ARQUIVOS .TXT */
char headers[4][120] = {"***************************************************SRTN***************************************************",
"************************************************GUARANTEED************************************************",
"**************************************************LOTTERY*************************************************",
"************************************************FAIR-SHARE************************************************"};

/* CABECALHO DO DIAGRAMA DO ARQUIVO DE SAIDA */
void imprimeCabecalho(void){
   printf("tempo   P01  P02  P03  P04  P05  P06  P07  P08  P09  P10  P11  P12  P13  P14  P15  P16  P17  P18  P19  P20\n");
}

/* ADICIONA PROCESSO AO BUFFER */
bool addProc(int id){
   if(((end+1)%BUFFERSIZE) != start){
      buffer[end].identificador = lista[id-1].identificador;
      buffer[end].dataCriacao = lista[id-1].dataCriacao;
      buffer[end].duracao = lista[id-1].duracao;
      buffer[end].tempoCorrido = 0;
      buffer[end].prioridade = lista[id-1].prioridade;
      buffer[end].estado = PRONTO;
      lista[id-1].estado = PRONTO;
      end = (end+1) % BUFFERSIZE;
      processosAtivos++;
      return true;
   }
   return false;
}

/* FINALIZA O PROCESSO */
void endProc(void){
    buffer[start].estado = FIM;
    int iterador_de_um_id = maptolist[buffer[start].identificador];
    lista[ iterador_de_um_id ].estado = FIM;
    processosAtivos--;
    start = (start + 1) % BUFFERSIZE;
}

/* LE PROCESSOS DO ARQUIVO E COLOCA NA LISTA*/
void inicializaListaProcessos(void){
   int i, j;
   Processo temp;
   for(i=0; i<MAX_PROCESSES; i++){
      lista[i].identificador = i+1;
      scanf("%d", &lista[i].dataCriacao);
      scanf("%d", &lista[i].duracao);
      scanf("%d", &lista[i].prioridade);
      lista[i].estado = NOVO;
   }

   //ORDENA OS PROCESSOS EM ORDEM DE DATA DE CRIACAO
   for(i=0; i<MAX_PROCESSES; i++){
      for(j=i+1; j<MAX_PROCESSES; j++){
         if(lista[i].dataCriacao>lista[j].dataCriacao){
            temp = lista[i];
            lista[i] = lista[j];
            lista[j] = temp;
         }
      }
   }
   for(i=0; i<MAX_PROCESSES; i++){
      maptolist[lista[i].identificador] = i;
   }
}

/* VERIFICA SE O PROCESSO FOI CRIADO E SE TEM ESPAÇO NO BUFFER */
void criaProcesso(){
    if(startFila == MAX_PROCESSES) return;
    while(lista[startFila].dataCriacao <= t && startFila<MAX_PROCESSES){
        if(addProc(startFila+1)){
            startFila++;
        }
        else{
            return;
        }
    }
}

/* IMPRIME AS LINHAS DO DIAGRANO NOS ARQUIVOS DE SAIDA */
void imprimeLinha(void){
   int identificador_na_lista;
   printf("%2d-%2d   ", t, t+QUANTUM); //imprime o tempo
   int id_do_processo_atual = buffer[start].identificador;
   for(identificador_na_lista=0; identificador_na_lista<MAX_PROCESSES; identificador_na_lista++){
      int iterador_de_um_id = maptolist[identificador_na_lista+1];
      if(identificador_na_lista+1==id_do_processo_atual)
         printf("###  ");
      else if(t < lista[iterador_de_um_id].dataCriacao || lista[iterador_de_um_id].estado==FIM) printf("     ");
      else printf("---  ");
   }
   printf("\n");
}

/* EXECUTA O PROCESSO */
void execute_next_process(void){
    buffer[start].estado = EXEC;
    buffer[start].duracao-=QUANTUM;
    buffer[start].tempoCorrido+=QUANTUM;
    if(buffer[start].duracao < 1) {
        QUANTUM+=buffer[start].duracao;
        endProc();
    }
}

/* INCREMENTA O TEMPO */
void clockTick(void){
    t+=QUANTUM;
}

/* ESCALONADOR SRTN*/
void srtn(void){
    QUANTUM = 1;
    int next = start;
    int j = (start+1)%BUFFERSIZE;

    while(j!=end){
        if(buffer[j].duracao < buffer[next].duracao){
            next = j;
        }else if((buffer[j].duracao == buffer[next].duracao) && (buffer[j].prioridade > buffer[next].prioridade)){
            next = j;
        }
        j = (j+1)%BUFFERSIZE;
    }
    Processo tempProc = buffer[next];
    buffer[next] = buffer[start];
    buffer[start] = tempProc;
}

/* ESCALONADOR GARANTIDO */
void guaranteed(void){
    QUANTUM = TIME_PER_PROCESS;
    int next = start;
    int j = (start+1)%BUFFERSIZE;
    int maiorDesdeCriacao = t - buffer[next].dataCriacao;
    float menorRazao;
    if(maiorDesdeCriacao == 0)
      menorRazao = 5000.0;
    else
      menorRazao = ((float)(buffer[next].tempoCorrido))/maiorDesdeCriacao;
    while(j!=end){
         //printf("%f %d\n", menorRazao, maiorDesdeCriacao);
        int desdeCriacao = t - buffer[j].dataCriacao;
        if(desdeCriacao == 0){
            j = (j+1)%BUFFERSIZE;
            continue;
        }
        float razao = ((float)(buffer[j].tempoCorrido))/desdeCriacao;
        if( fabs(razao) < 0.0001 && fabs(menorRazao) < 0.0001  && desdeCriacao > maiorDesdeCriacao) { // verifica se a razao e a menor razao são zero
            //se forem, verifica tb se a criacao do atual é anterior ao da menor razao
            //se for, proximo é o j
            maiorDesdeCriacao = desdeCriacao;
            menorRazao = razao;
            next = j;
        }
        else if(razao < menorRazao && (menorRazao - razao) > 0.0001){
            menorRazao = razao;
            maiorDesdeCriacao = desdeCriacao;
            next = j;
        }
        j = (j+1)%BUFFERSIZE;
    }
    Processo tempProc = buffer[next];
    buffer[next] = buffer[start];
    buffer[start] = tempProc;
}

/* ESCALONADOR LOTERIA */
void lottery(void){
    QUANTUM = 1;
    if(QUANTUM == 0) QUANTUM = 1;
    int next = start;
    int j = (start)%BUFFERSIZE;

    int maior = 0;
    while(j!=end){
        int prioridade = buffer[j].prioridade;
        int maiorLocal = 0;
        while(prioridade--){
            maiorLocal = rand()%10000;
            if(maiorLocal>maior){
                maior = maiorLocal;
                next = j;
            }
        }
        j = (j+1)%BUFFERSIZE;
    }
    Processo tempProc = buffer[next];
    buffer[next] = buffer[start];
    buffer[start] = tempProc;
}

/* ESCALONADOR FAIR-SHARE */
void fairShare(void){
    if(processosAtivos == 0) return;
    QUANTUM = CPU_TIME/processosAtivos;
    if(QUANTUM == 0) QUANTUM = 1;
    int next = start;
    int j = (start+1)%BUFFERSIZE;
    int maiorDesdeCriacao = t - buffer[next].dataCriacao;
    float menorRazao;
    if(maiorDesdeCriacao == 0)
      menorRazao = 5000.0;
    else
      menorRazao = buffer[next].prioridade*((float)(buffer[next].tempoCorrido))/maiorDesdeCriacao;
    while(j!=end){
         //printf("%f %d\n", menorRazao, maiorDesdeCriacao);
        int desdeCriacao = t - buffer[j].dataCriacao;
        if(desdeCriacao == 0){
            j = (j+1)%BUFFERSIZE;
            continue;
        }
        float razao = buffer[j].prioridade*((float)(buffer[j].tempoCorrido))/desdeCriacao;
        if( fabs(razao) < 0.0001 && fabs(menorRazao) < 0.0001  && desdeCriacao > maiorDesdeCriacao) { // verifica se a razao e a menor razao são zero
            //se forem, verifica tb se a criacao do atual é anterior ao da menor razao
            //se for, proximo é o j
            maiorDesdeCriacao = desdeCriacao;
            menorRazao = razao;
            next = j;
        }
        else if(razao < menorRazao && (menorRazao - razao) > 0.0001){
            menorRazao = razao;
            maiorDesdeCriacao = desdeCriacao;
            next = j;
        }
        j = (j+1)%BUFFERSIZE;
    }
    Processo tempProc = buffer[next];
    buffer[next] = buffer[start];
    buffer[start] = tempProc;
}

/* EXECUCAO DO KERNEL */
void loopKernel(){
    for(int kernel = 0; kernel<4;kernel++){//RODA PARA CADA UM DOS 4 ESCALONADORES
        FILE *fp;
        char saida[50] = "stdout";
        char numero[10];
        sprintf(numero, "%d", kernel + 1);
        strcat(saida, numero);
        strcat(saida, ".txt");
        fp = freopen(saida, "w+", stdout);
        t = 0;
        start = 0;
        end = 0;
        startFila = 0;

        printf("%s\n", headers[kernel]);
        imprimeCabecalho();

        while(1){
            criaProcesso();
            kernels[kernel].scheduler();
            bool bufferVazio = buffer[start].estado == FIM;
            bool filaVazia = startFila > MAX_PROCESSES-1;
            if(bufferVazio && filaVazia) break;
            if(bufferVazio) {
                continue;
            }
            imprimeLinha();
            execute_next_process();
            clockTick();
        }
        printf("\n");
        fclose(fp);
    }
}

int main(){
   srand(time(NULL)); //SEED PARA O LOTTERY
   freopen("stdin.txt", "r", stdin); //ABRE O ARQUIVO DE PROCESSOS

   /* DEFINIÇÃO DOS ESCALONADORES NO KERNEL */
   kernels[0].scheduler = srtn;
   kernels[1].scheduler = guaranteed;
   kernels[2].scheduler = lottery;
   kernels[3].scheduler = fairShare;

   inicializaListaProcessos(); //FAZ A LEITURA DOS PROCESSOS DO ARQUIVO .TXT
   loopKernel(); //INICIALIZA O KERNEL

   return 0;
}
