#!/bin/bash
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --job-name=t3-seq
#SBATCH --output=resultados/seq-%j.out
# Referências sequenciais, 10 repetições, em um nó do Atlântica.
#   N=<tamanho> sbatch scripts/seq.sh
cd "$SLURM_SUBMIT_DIR"; : "${N:?defina N}"; REPS=${REPS:-10}
echo "# $(date) host=$(hostname) N=$N"; lscpu | egrep 'Model name|Socket|Core|Thread|^CPU\(s\)'
for r in $(seq $REPS); do ./sequencial $N; done                            # T1
for P in 3 7 15 31 63 127 255 511 1023; do                                  # T1delta p/ versão original
  D=$(( (2*N + P) / (P+1) ))
  for r in $(seq $REPS); do ./sequencial $N $D; done
done
for P in 2 4 8 16 31 32; do                                                 # árvore da versão balanceada
  for r in $(seq $REPS); do ./sequencial $N -p $P; done
done
