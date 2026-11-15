#!/bin/bash
# Passo 1 (num nó do Atlântica, via sbatch/srun): acha N com T1 entre 30 e 60 s.
# Bubble Sort é quadrático: T(2n) ~ 4 T(n). Ajuste a lista de N conforme os tempos.
#   sbatch --nodes=1 --exclusive scripts/calibra.sh
#SBATCH --nodes=1
#SBATCH --exclusive
cd "$(dirname "$0")/.." 2>/dev/null || cd "$SLURM_SUBMIT_DIR"
for N in 100000 200000 300000 400000 500000; do ./sequencial $N; done
