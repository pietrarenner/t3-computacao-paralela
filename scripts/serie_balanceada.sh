#!/bin/bash
# Série: versão balanceada, P = 2 4 8 16 32 31 (10 repetições por configuração, definidas em config.sh)
# Uso: bash scripts/serie_balanceada.sh            -> todos os P
#      bash scripts/serie_balanceada.sh "16 32 31" -> só esses P
# Submete um job por vez, esperando vaga (o cluster limita os jobs na fila por usuário).
# Saída: resultados/balanceada-P<P>-<jobid>.out
cd "$(dirname "$0")/.."; source scripts/config.sh; : "${N:?edite N em scripts/config.sh}"
mkdir -p resultados
PS=${1:-"2 4 8 16 32 31"}; MAX=${MAXJOBS:-2}
OPT="--nodes=2 --nodelist=$NODELIST ${PART:+--partition=$PART}"
for P in $PS; do
  while [ "$(squeue -u $USER -h | wc -l)" -ge "$MAX" ]; do sleep 20; done
  VERSAO=balanceada sbatch -J bal-P$P -o resultados/balanceada-P$P-%j.out $OPT --ntasks=$P scripts/job.sh
done