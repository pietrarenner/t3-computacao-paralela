#!/bin/bash
# Compila tudo com as MESMAS flags (-O2). No Atlântica: module load do Open MPI antes, se necessário.
set -e
cd "$(dirname "$0")/.."
mpicc -O2 -o original original.c
mpicc -O2 -o balanceada balanceada.c
gcc   -O2 -o sequencial sequencial.c
echo "compilado"
