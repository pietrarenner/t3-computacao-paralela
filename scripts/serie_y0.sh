#!/bin/bash
# Série: original, P = 63 127 255 511 1023, yield_when_idle=0 (espera ativa), 10 repetições (config.sh)
# Uso: bash scripts/serie_y0.sh            -> todos os P
#      bash scripts/serie_y0.sh "511 1023" -> só esses P
# Submete um job por vez, esperando vaga (o cluster limita os jobs na fila por usuário).
cd "$(dirname "$0")/.."; source scripts/config.sh; : "${N:?edite N em scripts/config.sh}"
mkdir -p resultados
PS=${1:-"63 127 255 511 1023"}; MAX=${MAXJOBS:-2}
OPT="--nodes=2 --nodelist=$NODELIST ${PART:+--partition=$PART}"
for P in $PS; do
  while [ "$(squeue -u $USER -h | wc -l)" -ge "$MAX" ]; do sleep 20; done
  VERSAO=original YIELD=0 sbatch $OPT --ntasks=$P scripts/job.sh
done