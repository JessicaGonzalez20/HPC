#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <math.h>
#include <time.h>

#define MAX_ITER 10000
#define TOL 1e-6

double *u, *u_new;
int N, NUM_PROCESOS;
double h;

/*----------------------------------
POISSON PARCIAL (PROCESO)
----------------------------------*/
void poisson_parcial(int inicio, int fin){

    for(int k=0;k<MAX_ITER;k++){

        for(int i=inicio;i<fin;i++){
            if(i>0 && i<N-1)
                u_new[i] = 0.5 * (u[i-1] + u[i+1] - h*h * sin(3.141592653589793 * i*h));
        }

        for(int i=inicio;i<fin;i++){
            if(i>0 && i<N-1)
                u[i] = u_new[i];
        }
    }
}

/*----------------------------------
MAIN
----------------------------------*/
int main(int argc,char *argv[]){

    if(argc < 3){
        printf("Uso: ./poisson_proc N PROCESOS\n");
        return 1;
    }

    N = atoi(argv[1]);
    NUM_PROCESOS = atoi(argv[2]);

    h = 1.0/(N-1);

    /* MEMORIA COMPARTIDA */
    u = mmap(NULL,N*sizeof(double),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    u_new = mmap(NULL,N*sizeof(double),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);

    for(int i=0;i<N;i++){
        u[i] = 0.0;
        u_new[i] = 0.0;
    }

    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    int bloque = N / NUM_PROCESOS;

    for(int p=0;p<NUM_PROCESOS;p++){

        pid_t pid = fork();

        if(pid == 0){

            int ini = p * bloque;
            int fin_p = (p==NUM_PROCESOS-1)?N:(p+1)*bloque;

            poisson_parcial(ini, fin_p);
            exit(0);
        }
    }

    for(int p=0;p<NUM_PROCESOS;p++)
        wait(NULL);

    clock_gettime(CLOCK_MONOTONIC, &fin);

    double tiempo =
        (fin.tv_sec - inicio.tv_sec) +
        (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("N=%d Procesos=%d Tiempo=%f\n", N, NUM_PROCESOS, tiempo);

    return 0;
}