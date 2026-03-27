#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h> // Para la memoria compartida
#include <math.h>
#include <time.h>

#define MAX_ITER 10000
#define TOL 1e-6

double *u, *u_new;
int N, NUM_PROCESOS;
double h;


/*usamos fork para "clonar" el programa y que cada copia (proceso) 
se encargue de procesar una parte distinta de los datos al mismo tiempo. 
Como normalmente los procesos son independientes y no pueden ver lo que 
hace el otro, creamos una zona de memoria compartida con mmap que funciona 
como una "pizarra común" donde todos leen y escriben sus avances. Al final, 
el proceso principal espera a que todos terminen su pedazo del trabajo para 
detener el cronómetro y ver cuánto tiempo ahorramos al trabajar en equipo.*/

/*----------------------------------
 TRABAJO POR CADA PROCESO
----------------------------------*/
void poisson_parcial(int inicio, int fin){
    // Cada trabajador repite el proceso miles de veces
    for(int k=0;k<MAX_ITER;k++){

        // Paso 1: Calcular los nuevos valores del "pedazo" que le tocó
        for(int i=inicio;i<fin;i++){
            if(i>0 && i<N-1)
                u_new[i] = 0.5 * (u[i-1] + u[i+1] - h*h * sin(3.141592653589793 * i*h));
        }

        // Paso 2: Actualizar los valores viejos con los nuevos
        for(int i=inicio;i<fin;i++){
            if(i>0 && i<N-1)
                u[i] = u_new[i];
        }
    }
}

/*----------------------------------
 FUNCIÓN PRINCIPAL
----------------------------------*/
int main(int argc,char *argv[]){

    // Revisamos que el usuario nos diga el tamaño (N) y cuántos procesos usar
    if(argc < 3){
        printf("Uso: ./poisson_proc N PROCESOS\n");
        return 1;
    }

    N = atoi(argv[1]);
    NUM_PROCESOS = atoi(argv[2]);

    h = 1.0/(N-1);

    /* CREAR LA "MESA COMÚN" (MEMORIA COMPARTIDA) 
       Usamos mmap para que todos los procesos puedan ver y editar u y u_new */
    u = mmap(NULL, N*sizeof(double), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    u_new = mmap(NULL, N*sizeof(double), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    // Llenamos todo de ceros al empezar
    for(int i=0;i<N;i++){
        u[i] = 0.0;
        u_new[i] = 0.0;
    }

    // Empezamos a cronometrar
    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // Calculamos qué tan grande es el "pedazo" de cada proceso
    int bloque = N / NUM_PROCESOS;

    /* CREAR A LOS TRABAJADORES */
    for(int p=0;p<NUM_PROCESOS;p++){

        pid_t pid = fork(); // Aquí el programa se "clona"

        if(pid == 0){ // Si soy un hijo (trabajador)...
            
            // Calculo desde dónde hasta dónde me toca trabajar
            int ini = p * bloque;
            int fin_p = (p==NUM_PROCESOS-1)?N:(p+1)*bloque;

            // Hago mi parte del trabajo
            poisson_parcial(ini, fin_p);
            
            exit(0); // Termino mi labor y me cierro
        }
    }

    /* EL JEFE ESPERA */
    for(int p=0;p<NUM_PROCESOS;p++)
        wait(NULL); // Espera a que todos los hijos terminen

    // Paramos el cronómetro
    clock_gettime(CLOCK_MONOTONIC, &fin);

    double tiempo = (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("N=%d Procesos=%d Tiempo=%f\n", N, NUM_PROCESOS, tiempo);

    return 0;
}
