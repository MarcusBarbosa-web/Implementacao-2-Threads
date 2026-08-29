#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


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
        printf("Falha ao abrir o arquivo\n");
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
        printf("Falha ao abrir o arquivo\n");
        return;
    }

    fprintf(arq, "%f\n", tempo);
    
    fclose(arq);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

int *calcula_mandelbrot_serial(int largura, int altura, int maximodeinteracoes){
    int *resultado;

    resultado = malloc(largura * altura * sizeof(int));

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

    resultado = malloc(largura * altura * sizeof(int));

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(){
    int largura = 400, altura = 400;
    int maximodeinteracoes = 100;
    int primeiraescrita = 1;

    struct timespec inicio, fim;

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    int *resultado = calcula_mandelbrot_serial(largura, altura, maximodeinteracoes);
    clock_gettime(CLOCK_MONOTONIC, &fim);

    double tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;

    escreverno_pgm(resultado, "mandelbrot_mvpb_serial.pgm", largura, altura);
    escreverno_txt(tempo_total, "times.txt", primeiraescrita);
    
    free(resultado);



    primeiraescrita = 0;

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    resultado = calcula_mandelbrot_openmp(largura, altura, maximodeinteracoes);
    clock_gettime(CLOCK_MONOTONIC, &fim);

    tempo_total = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;

    escreverno_pgm(resultado, "mandelbrot_mvpb_openmp.pgm", largura, altura);
    escreverno_txt(tempo_total, "times.txt", primeiraescrita);
    
    free(resultado);

    return 0;
}