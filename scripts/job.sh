#!/bin/bash
#SBATCH --nodes=2
#SBATCH --exclusive
#SBATCH --overcommit
#SBATCH --job-name=t3
#SBATCH --output=resultados/%x-%j.out
# Uso (ntasks vai na linha de comando):
#   N=<n> VERSAO=original|balanceada|balanceada_send [YIELD=0|1] sbatch --ntasks=P scripts/job.sh
cd "$SLURM_SUBMIT_DIR"
: "${N:?}"; : "${VERSAO:?}"; REPS=${REPS:-10}; P=$SLURM_NTASKS
[ -n "$YIELD" ] && export OMPI_MCA_mpi_yield_when_idle=$YIELD   # definido explicitamente
echo "# $(date) versao=$VERSAO P=$P N=$N yield=${YIELD:-nd} nodes=$SLURM_JOB_NODELIST"
echo "# distribuicao (tarefas por no):"; srun --ntasks=$P hostname | sort | uniq -c
echo "# rank -> no:"; srun --ntasks=$P bash -c 'echo "rank=$SLURM_PROCID no=$(hostname)"' | sort -t= -k2 -n
for r in $(seq $REPS); do
  if [ "$VERSAO" = original ]; then
    D=$(( (2*N + P) / (P+1) ))                 # delta = ceil(2n/(P+1))
    echo "REP $r yield=${YIELD:-nd}"; srun ./original $N $D
  else
    echo "REP $r"; srun ./$VERSAO $N              # balanceada ou balanceada_send
  fi
done