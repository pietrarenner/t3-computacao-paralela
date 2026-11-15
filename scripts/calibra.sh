#!/bin/bash
# Passo 1 (num nó do Atlântica, via sbatch/srun): acha N com T1 entre 30 e 60 s.
# Bubble Sort é quadrático: T(2n) ~ 4 T(n). Ajuste a lista de N conforme os tempos.
#   sbatch scripts/calibra.sh      (da raiz do repositório; saída em calibra-<jobid>.out)
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --output=calibra-%j.out
cd "${SLURM_SUBMIT_DIR:-$(dirname "$0")/..}"

RESULTADOS=""
for N in 150000 200000 225000 250000 275000 300000; do
    LINHA=$(./sequencial $N)
    echo "$LINHA"                                   # saída bruta, aparece no .out
    T=$(echo "$LINHA" | sed -n 's/.*tempo=\([0-9.]*\).*/\1/p')
    RESULTADOS="$RESULTADOS$N $T"$'\n'
done

# Resumo para escolher N: marca com <== os que ficam entre 30 e 60 s.
echo
echo "===== RESUMO (T1 alvo: 30 a 60 s) ====="
printf "%-10s %-12s\n" "N" "T1 (s)"
echo -n "$RESULTADOS" | awk '{ printf "%-10s %-12.3f %s\n", $1, $2, ($2>=30 && $2<=60) ? "<== OK" : "" }'
