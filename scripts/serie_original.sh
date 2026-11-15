#!/bin/bash
# Série: versão original, P = 1 3 7 15 31 (10 repetições por configuração, definidas em config.sh)
# Uso: bash scripts/serie_original.sh            -> todos os P
#      bash scripts/serie_original.sh "7 15 31"  -> só esses P
# Submete um job por vez, esperando vaga (o cluster limita os jobs na fila por usuário).
cd "$(dirname "$0")/.."; source scripts/config.sh; : "${N:?edite N em scripts/config.sh}"
mkdir -p resultados
PS=${1:-"1 3 7 15 31"}; MAX=${MAXJOBS:-2}
OPT="--nodes=2 --nodelist=$NODELIST ${PART:+--partition=$PART}"
for P in $PS; do
  while [ "$(squeue -u $USER -h | wc -l)" -ge "$MAX" ]; do sleep 20; done
  VERSAO=original sbatch $OPT --ntasks=$P scripts/job.sh
done