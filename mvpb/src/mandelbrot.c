#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>

typedef struct node{
    int linhainicio;
    int linhafim;
    int largura;
    int altura;
    int maximodeinteracoes;
    int *resultado;
}node;

typedef struct node2{
    int largura;
    int altura;
    int maximodeinteracoes;
    int *resultado;
}node2;

int proxima_linha = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void escreve_erronotxt(char *escreve){
    FILE *novo = fopen("falha.txt", "a");

    if (novo != NULL){
        fprintf(novo, "%s\n", escreve);

        fclose(novo);
    }
}


int calcula_pixel_serial(int j, int i, double largura, double altura, int maximodeinteracoes){
    double cr = -2.0 + (j/(double)largura) * 3.0;
    double ci = -1.5 + (i/(double)altura) * 3.0;

    double realz = 0;
    double zi = 0;
    int cont = 0;
    double aux_re = 0;
    double aux_ir = 0;

    while(cont < maximodeinteracoes && (realz*realz + zi*zi) <= 4){
        aux_re = (realz*realz) - (zi*zi) + cr;
        aux_ir = 2 * (realz * zi) + ci;

        realz = aux_re;
        zi = aux_ir;
        cont++;
    }

    return cont;
}

void escreverno_pgm(int *resultado, char *arquivo, int largura, int altura){

    FILE *arq = fopen(arquivo, "w");

    if (arq == NULL){
        escreve_erronotxt("Falha ao abrir o arquivo .pgm");
        return;
    }

    for(int i = 0; i < altura; i++){
        for (int j = 0; j < largura; j++){
            int valor = resultado[i * largura + j];

            fprintf(arq, "%d", valor);

            if(j == largura-1){
                fprintf(arq, "\n");
            }else{
                fprintf(arq, " ");
            }

        }
    }
    
    fclose(arq);
}

void escreverno_txt(double tempo, char *arquivo, int primeiraescrita){

    FILE *arq;

    if(primeiraescrita == 1){
        arq = fopen(arquivo, "w");
    }else{
        arq = fopen(arquivo, "a");
    }

    if (arq == NULL){
        escreve_erronotxt("Falha ao abrir o arquivo .txt");
        return;
    }

    fprintf(arq, "%f\n", tempo);
    
    fclose(arq);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

int *calcula_mandelbrot_serial(int largura, int altura, int maximodeinteracoes){
    int *resultado;

    resultado = malloc((size_t)largura * altura * sizeof(int));
    if(resultado == NULL){
        escreve_erronotxt("Falha em alocar memoria");
        return NULL;   
    }

    for (int i = 0; i < altura; i++){
        for (int j = 0; j < largura; j++){
            int contador = calcula_pixel_serial(j, i, largura, altura, maximodeinteracoes);
            int intensidade = (int)(((double)contador / maximodeinteracoes) * 255);

            resultado[i * largura + j] = intensidade;
        }
    }

    return resultado;
}

int *calcula_mandelbrot_openmp(int largura, int altura, int maximodeinteracoes){
    int *resultado;

    resultado = malloc((size_t)largura * altura * sizeof(int));
    if(resultado == NULL){
        escreve_erronotxt("Falha em alocar memoria");
        return NULL;   
    }

    #pragma omp parallel for

    for (int i = 0; i < altura; i++){
        for (int j = 0; j < largura; j++){
            int contador = calcula_pixel_serial(j, i, largura, altura, maximodeinteracoes);
            int intensidade = (int)(((double)contador / maximodeinteracoes) * 255);

            resultado[i * largura + j] = intensidade;
        }
    }

    return resultado;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void *funcao_thread(void *argumento){

    node *novo = (node*) argumento;

    int auxfim = novo->linhafim;
    int auxlargura = novo->largura;

    for (int i = novo->linhainicio; i < auxfim; i++){
        for (int j = 0; j < auxlargura; j++){
            int contador = calcula_pixel_serial(j, i, novo->largura, novo->altura, novo->maximodeinteracoes);
            int intensidade = (int)(((double)contador / novo->maximodeinteracoes) * 255);

            novo->resultado[i * auxlargura + j] = intensidade;
        }
    }

    return NULL;
}

int *calcula_mandelbrot_thread(int largura, int altura, int maximodeinteracoes, int numthreads){
    int *resultado = malloc((size_t)largura * altura * sizeof(int));
    if(resultado == NULL){
        escreve_erronotxt("Falha em alocar memoria");
        return NULL;   
    }

    int linhasthread = altura/numthreads;
    int linhassobra = altura % numthreads;

    node *dados = malloc(numthreads * sizeof(node));
    if(dados == NULL){
        escreve_erronotxt("Falha em alocar memoria");
        free(resultado);
        return NULL;   
    }

    pthread_t *threads = malloc(numthreads * sizeof(pthread_t));
    if(threads == NULL){
        escreve_erronotxt("Falha em alocar memoria");
        free(resultado);
        free(dados);
        return NULL;   
    }

    for (int i = 0; i < numthreads; i++){
        dados[i].linhainicio = i * linhasthread;
        dados[i].linhafim = dados[i].linhainicio + linhasthread;

        if(i == numthreads - 1){
            dados[i].linhafim = dados[i].linhafim + linhassobra;
        }

        dados[i].largura = largura;
        dados[i].altura = altura;
        dados[i].maximodeinteracoes = maximodeinteracoes;
        dados[i].resultado = resultado;

        int resultado_create = pthread_create(&threads[i], NULL, funcao_thread, &dados[i]);
        if (resultado_create != 0){
            escreve_erronotxt("Falha na criacao de um thread");

            for (int k = 0; k < i; k++){
                pthread_join(threads[k], NULL);
            }

            free(resultado);
            free(dados);
            free(threads);
            return NULL;
        } 
    }

    for (int i = 0; i < numthreads; i++){
        pthread_join(threads[i], NULL);
    }

    free(dados);
    free(threads);

    return resultado;
}





void *funcao_thread2(void *argumento){
    node2 *novo = (node2*) argumento;

    while(1){

        pthread_mutex_lock(&mutex);

        int linha_atual = proxima_linha;
        proxima_linha++;

        pthread_mutex_unlock(&mutex);

        if (linha_atual >= novo->altura){
            break;
        }

        for (int j = 0; j < novo->largura; j++){
            int contador = calcula_pixel_serial(j, linha_atual, novo->largura, novo->altura, novo->maximodeinteracoes);
            int intensidade = (int)(((double)contador / novo->maximodeinteracoes) * 255);

            novo->resultado[linha_atual * novo->largura + j] = intensidade;
        }

    }
    return NULL;
}

int *calcula_mandelbrot_thread2(int largura, int altura, int maximodeinteracoes, int numthreads){
    proxima_linha = 0;

    int *resultado = malloc((size_t)largura * altura * sizeof(int));
    if(resultado == NULL){
        escreve_erronotxt("Falha em alocar memoria");
        return NULL;   
    }

    node2 *dados = malloc(numthreads * sizeof(node2));
    if(dados == NULL){
        escreve_erronotxt("Falha em alocar memoria");
        free(resultado);
        return NULL;   
    }

    pthread_t *threads = malloc(numthreads * sizeof(pthread_t));
    if(threads == NULL){
        escreve_erronotxt("Falha em alocar memoria");
        free(resultado);
        free(dados);
        return NULL;   
    }

    for (int i = 0; i < numthreads; i++){

        dados[i].largura = largura;
        dados[i].altura = altura;
        dados[i].maximodeinteracoes = maximodeinteracoes;
        dados[i].resultado = resultado;

        int resultado_create = pthread_create(&threads[i], NULL, funcao_thread2, &dados[i]);
        if (resultado_create != 0){
            escreve_erronotxt("Falha na criacao de um thread");

            for (int k = 0; k < i; k++){
                pthread_join(threads[k], NULL);
            }

            free(resultado);
            free(dados);
            free(threads);
            return NULL;
        }
    }

    for (int i = 0; i < numthreads; i++){
        pthread_join(threads[i], NULL);
    }

    free(dados);
    free(threads);

    return resultado;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[]){
    if(argc != 5){
        escreve_erronotxt("Numero de parametros incorretos passados");
        return 1;
    }

    char *entradainvalida;
//////
    long larguralong = strtol(argv[1], &entradainvalida, 10);

    if (*entradainvalida != '\0'){
        escreve_erronotxt("Entrada invalidada nos argumentos da largura");
        return 1;
    }
    if (larguralong <= 0 || larguralong > INT_MAX){
        escreve_erronotxt("Numero da largura incorreto");
        return 1;
    }

    int largura = (int) larguralong;
//////
    long alturalong = strtol(argv[2], &entradainvalida, 10);

    if (*entradainvalida != '\0'){
        escreve_erronotxt("Entrada invalidada nos argumentos da altura");
        return 1;
    }
    if (alturalong <= 0 || alturalong > INT_MAX){
        escreve_erronotxt("Numero da altura incorreto");
        return 1;
    }

    int altura = (int) alturalong;
//////
    long maximodeinteracoeslong = strtol(argv[3], &entradainvalida, 10);

    if (*entradainvalida != '\0'){
        escreve_erronotxt("Entrada invalidada nos argumentos de maximo de interacoes");
        return 1;
    }
    if (maximodeinteracoeslong <= 0 || maximodeinteracoeslong > INT_MAX){
        escreve_erronotxt("Numero maximo de interacoes incorreto");
        return 1;
    }

    int maximodeinteracoes = (int) maximodeinteracoeslong;
//////
    long numthreadslong = strtol(argv[4], &entradainvalida, 10);

    if (*entradainvalida != '\0'){
        escreve_erronotxt("Entrada invalidada nos argumentos do numero de threads");
        return 1;
    }
    if (numthreadslong <= 0 || numthreadslong > INT_MAX){
        escreve_erronotxt("Numero de threads incorreto");
        return 1;
    }

    int numthreads = (int) numthreadslong;
//////

    int primeiraescrita = 1;
    struct timespec inicio, fim;

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    int *resultado = calcula_mandelbrot_serial(largura, altura, maximodeinteracoes);
    if (resultado == NULL){
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);

    double tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;

    escreverno_pgm(resultado, "mandelbrot_mvpb_serial.pgm", largura, altura);
    escreverno_txt(tempo_total, "times.txt", primeiraescrita);
    
    free(resultado);



    primeiraescrita = 0;

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    resultado = calcula_mandelbrot_openmp(largura, altura, maximodeinteracoes);
    if (resultado == NULL){
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);

    tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;

    escreverno_pgm(resultado, "mandelbrot_mvpb_openmp.pgm", largura, altura);
    escreverno_txt(tempo_total, "times.txt", primeiraescrita);
    
    free(resultado);



    primeiraescrita = 0;

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    resultado = calcula_mandelbrot_thread(largura, altura, maximodeinteracoes, numthreads);
    if (resultado == NULL){
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);

    tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;

    escreverno_pgm(resultado, "mandelbrot_mvpb_pthreads1.pgm", largura, altura);
    escreverno_txt(tempo_total, "times.txt", primeiraescrita);
    
    free(resultado);



    primeiraescrita = 0;

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    resultado = calcula_mandelbrot_thread2(largura, altura, maximodeinteracoes, numthreads);
    if (resultado == NULL){
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);

    tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;

    escreverno_pgm(resultado, "mandelbrot_mvpb_pthreads2.pgm", largura, altura);
    escreverno_txt(tempo_total, "times.txt", primeiraescrita);
    
    free(resultado);

    return 0;
}