#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#define MAX_ITER 10000
#define TOL 1e-6

double *u, *u_new;
int N, NUM_HILOS;
double h;

typedef struct{
    int inicio;
    int fin;
} DatosHilo;

pthread_barrier_t barrera;

/*----------------------------------*/
void *poisson_paralelo(void *arg){

    DatosHilo *d = (DatosHilo*)arg;

    for(int k=0;k<MAX_ITER;k++){

        for(int i=d->inicio;i<d->fin;i++){
            if(i>0 && i<N-1)
                u_new[i] = 0.5 * (u[i-1] + u[i+1] - h*h * sin(3.141592653589793 * i*h));
        }

        pthread_barrier_wait(&barrera);

        for(int i=d->inicio;i<d->fin;i++){
            if(i>0 && i<N-1)
                u[i] = u_new[i];
        }

        pthread_barrier_wait(&barrera);
    }

    return NULL;
}

/*----------------------------------*/
int main(int argc, char *argv[]){

    if(argc < 3){
        printf("Uso: ./poisson_thr N HILOS\n");
        return 1;
    }

    N = atoi(argv[1]);
    NUM_HILOS = atoi(argv[2]);

    u = malloc(N*sizeof(double));
    u_new = malloc(N*sizeof(double));

    h = 1.0/(N-1);

    for(int i=0;i<N;i++){
        u[i] = 0.0;
        u_new[i] = 0.0;
    }

    pthread_t hilos[NUM_HILOS];
    DatosHilo datos[NUM_HILOS];

    pthread_barrier_init(&barrera, NULL, NUM_HILOS);

    int bloque = N / NUM_HILOS;

    clock_t inicio = clock();

    for(int i=0;i<NUM_HILOS;i++){
        datos[i].inicio = i * bloque;
        datos[i].fin = (i==NUM_HILOS-1)?N:(i+1)*bloque;

        pthread_create(&hilos[i], NULL, poisson_paralelo, &datos[i]);
    }

    for(int i=0;i<NUM_HILOS;i++)
        pthread_join(hilos[i], NULL);

    clock_t fin = clock();

    double tiempo = (double)(fin - inicio)/CLOCKS_PER_SEC;

    printf("N=%d Hilos=%d Tiempo=%f\n", N, NUM_HILOS, tiempo);

    pthread_barrier_destroy(&barrera);

    free(u);
    free(u_new);

    return 0;
}