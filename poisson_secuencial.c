#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_ITER 10000
#define TOL 1e-6

double *u, *u_new; /*Guarda dos arreglos (iteracion actual y la siguiente)*/
int N;
double h;

/*Este código calcula los valores de una fila de puntos basándose 
en una regla simple: el valor de cada punto debe ser el promedio 
de sus dos vecinos (el de la izquierda y el de la derecha). 
Para que las cuentas no se enreden, el programa usa dos listas de números: 
consulta los valores viejos en una y anota los nuevos en la otra, repitiendo 
este proceso miles de veces hasta que los números dejan de cambiar. */

void poisson_secuencial(){
    for(int k=0;k<MAX_ITER;k++){  /*criterios de parada*/
        for(int i=1;i<N-1;i++){
            u_new[i] = 0.5 * (u[i-1] + u[i+1] - h*h * sin(3.141592653589793 * i*h));
        }/*Stencil: Cada punto se calcula como el promedio de sus vecinos, restandole la funcion seno*/

        double error = 0.0;

        for(int i=1;i<N-1;i++){
            error += fabs(u_new[i] - u[i]);
            u[i] = u_new[i];
        }

        if(error < TOL) break;
    }
}

int main(int argc, char *argv[]){

    if(argc < 2){
        printf("Uso: ./poisson_sec N\n");
        return 1;
    }

    N = atoi(argv[1]);

    u = malloc(N*sizeof(double));
    u_new = malloc(N*sizeof(double));

    h = 1.0/(N-1);

    for(int i=0;i<N;i++){
        u[i] = 0.0;
        u_new[i] = 0.0;
    }

    clock_t inicio = clock();

    poisson_secuencial();

    clock_t fin = clock();

    double tiempo = (double)(fin - inicio)/CLOCKS_PER_SEC;

    printf("N=%d Tiempo=%f\n", N, tiempo);

    free(u);
    free(u_new);

    return 0;
}
