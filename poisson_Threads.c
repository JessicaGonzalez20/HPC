#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> // Librería para manejar hilos
#include <math.h>
#include <time.h>

#define MAX_ITER 10000
#define TOL 1e-6

double *u, *u_new;
int N, NUM_HILOS;
double h;

// Estructura para decirle a cada hilo qué parte de la fila le toca
typedef struct{
    int inicio;
    int fin;
} DatosHilo;

// La BARRERA: Es el semáforo que hace que todos los hilos se esperen
pthread_barrier_t barrera;

/*Este código usa hilos (Pthreads) para repartir el cálculo de Poisson 
entre varios núcleos de la CPU. A diferencia del código de procesos, 
aquí no necesitamos mmap porque los hilos comparten la memoria de forma natural. 
Sin embargo, para que el resultado sea correcto, usamos una barrera de 
sincronización (pthread_barrier_wait). Esta barrera obliga a que todos los 
hilos terminen de calcular antes de actualizar la lista, evitando que un 
hilo se adelante y use datos viejos de la vuelta anterior*/

/*----------------------------------
 TRABAJO DEL HILO
----------------------------------*/
void *poisson_paralelo(void *arg){
    DatosHilo *d = (DatosHilo*)arg;

    for(int k=0;k<MAX_ITER;k++){

        // 1. Cada hilo calcula su pedazo de la lista u_new
        for(int i=d->inicio;i<d->fin;i++){
            if(i>0 && i<N-1)
                u_new[i] = 0.5 * (u[i-1] + u[i+1] - h*h * sin(3.141592653589793 * i*h));
        }

        // ESPERA: Nadie puede pasar a actualizar u hasta que todos terminen de calcular u_new
        pthread_barrier_wait(&barrera);

        // 2. Cada hilo actualiza su pedazo de la lista original u
        for(int i=d->inicio;i<d->fin;i++){
            if(i>0 && i<N-1)
                u[i] = u_new[i];
        }

        // ESPERA: Nadie puede empezar la siguiente vuelta (k+1) hasta que todos hayan actualizado u
        pthread_barrier_wait(&barrera);
    }

    return NULL;
}

/*----------------------------------
 MAIN
----------------------------------*/
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

    // Inicializamos la barrera para el número de hilos que el usuario pidió
    pthread_barrier_init(&barrera, NULL, NUM_HILOS);

    int bloque = N / NUM_HILOS;

    clock_t inicio = clock();

    // CREACIÓN DE HILOS: Lanzamos a los trabajadores
    for(int i=0;i<NUM_HILOS;i++){
        datos[i].inicio = i * bloque;
        datos[i].fin = (i==NUM_HILOS-1)?N:(i+1)*bloque;

        pthread_create(&hilos[i], NULL, poisson_paralelo, &datos[i]);
    }

    // ESPERA FINAL: El programa principal espera a que todos terminen
    for(int i=0;i<NUM_HILOS;i++)
        pthread_join(hilos[i], NULL);

    clock_t fin = clock();

    double tiempo = (double)(fin - inicio)/CLOCKS_PER_SEC;
    printf("N=%d Hilos=%d Tiempo=%f\n", N, NUM_HILOS, tiempo);

    // Limpieza
    pthread_barrier_destroy(&barrera);
    free(u);
    free(u_new);

    return 0;
}
