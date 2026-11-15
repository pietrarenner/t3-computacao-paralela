#!/bin/bash
# Série: original, P = 63..1023, yield_when_idle=0 (espera ativa) (10 repetições por configuração, definidas em config.sh)
# Uso: bash scripts/serie_y0.sh   (da pasta do projeto, depois de editar scripts/config.sh)
cd "$(dirname "$0")/.."; source scripts/config.sh; : "${N:?edite N em scripts/config.sh}"
OPT="--nodes=2 --nodelist=$NODELIST ${PART:+--partition=$PART}"
for P in 63 127 255 511 1023; do VERSAO=original YIELD=0 sbatch $OPT --ntasks=$P scripts/job.sh; done
