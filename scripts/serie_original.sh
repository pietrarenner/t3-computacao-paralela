#!/bin/bash
# Série: versão original, P = 1 3 7 15 31 (10 repetições por configuração, definidas em config.sh)
# Uso: bash scripts/serie_original.sh   (da pasta do projeto, depois de editar scripts/config.sh)
cd "$(dirname "$0")/.."; source scripts/config.sh; : "${N:?edite N em scripts/config.sh}"
OPT="--nodes=2 --nodelist=$NODELIST ${PART:+--partition=$PART}"
for P in 1 3 7 15 31; do VERSAO=original sbatch $OPT --ntasks=$P scripts/job.sh; done
