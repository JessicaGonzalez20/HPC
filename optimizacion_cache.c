#!/bin/bash

echo "=============================="
echo "PRUEBAS CON OPTIMIZACIONES GCC"
echo "=============================="

for O in 0 1 2 3
do
  echo ""
  echo "--------------------------------"
  echo "Compilando con -O$O"
  echo "--------------------------------"

  gcc poisson_Threads.c -o thr_O$O -lpthread -lm -O$O

  echo ""
  echo "Ejecutando pruebas con -O$O"
  echo ""

  for N in 1000 2000 3000 4000 5000
  do
    for H in 2 4 6 8 10 12
    do
      echo "Optimizacion: -O$O | N=$N | Hilos=$H"
      ./thr_O$O $N $H
    done
  done
done
