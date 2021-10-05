#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define Produtores 1    //Modifique de acordo com a necessidade
#define Consumidores 1  //Modifique de acordo com a necessidade
#define BufferSize 5    //Modifique de acordo com a necessidade

//Variaveis globais
sem_t condProd, condCons; //Semaforos para sincronizar a ordem de execucao das threads
int buffer[BufferSize], count=0, in=0, NThreads = (Produtores + Consumidores);

void *Produtor(void *arg) {
    int *id = (int *) arg;
    printf("Sou a thread produtora %d\n", *id);
    while(1) {
        Insere(*id);
        sleep(1);
    }
    free(arg);
    pthread_exit(NULL);
}

void Insere(int id){
    int item = rand();
    sem_wait(&condProd);
    printf("P[%d] quer inserir %d\n", id, item);
    if(count == BufferSize) {
        printf("P[%d] bloqueou\n", id);
        sem_post(&condCons);
        sem_wait(&condProd);
        printf("P[%d] desbloqueou\n", id);
    }
    buffer[in] = item;
    in = (in + 1)%BufferSize;
    count++;
    printf("P[%d] inseriu\n", id);
    ImprimeBuffer(BufferSize);

    sem_post(&condProd);
}

void *Consumidor(void * arg) {
    int *id = (int *) arg;
    int item;
    printf("Sou a thread consumidora %d\n", *id);
    while(1) {
        Retira(*id);
        sleep(1);
    } 
    free(arg);
    pthread_exit(NULL);
}

void Retira(int id){
    sem_wait(&condCons);
    
    for (int i=0; i < BufferSize; i++){
        buffer[i] = 0;
    }

    count = 0;
    printf("C[%d] consumiu o buffer\n", id);
    ImprimeBuffer(BufferSize);

    sem_post(&condProd);
}

void ImprimeBuffer (int n){
    for (int i=0; i<n; i++){
        printf("%d ", buffer[i]);
    }
    printf("\n");
}

//Funcao principal
int main(int argc, char *argv[]) {
    pthread_t tid[NThreads];
    int *id[NThreads], i;

    //Alocando memoria
    for (i=0; i<(NThreads); i++) {
        if ((id[i] = malloc(sizeof(int))) == NULL) {
            pthread_exit(NULL); return 1;
        }
        *id[i] = i+1;
    }

    //Inicializando buffer
    for (i=0; i<BufferSize; i++){
        buffer[i] = 0;
    }

    //Iniciando os semaforos
    sem_init(&condProd, 0, 1);
    sem_init(&condCons, 0, 0);

    //Criando os produtores
    for (i=0; i<Produtores; i++){
        if (pthread_create(&tid[i], NULL, Produtor, (void *)id[i])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
    }

    //Criando os consumidores
    for (i=Produtores; i<(Produtores+Consumidores); i++){
        if (pthread_create(&tid[i], NULL, Consumidor, (void *)id[i])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
    }

    //Esperando todas as threads terminarem
    for (i=0; i<(Produtores+Consumidores); i++) {
        if (pthread_join(tid[i], NULL)) {
            printf("--ERRO: pthread_join() \n"); exit(-1); 
        }
        free(id[i]);
    }

    //Destruindo os semaforos
    sem_destroy(&condProd);
    sem_destroy(&condCons);

    pthread_exit(NULL);
}
