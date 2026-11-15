#!/bin/bash
# Série: versão balanceada, P = 2 4 8 16 32 31 (10 repetições por configuração, definidas em config.sh)
# Uso: bash scripts/serie_balanceada.sh   (da pasta do projeto, depois de editar scripts/config.sh)
cd "$(dirname "$0")/.."; source scripts/config.sh; : "${N:?edite N em scripts/config.sh}"
OPT="--nodes=2 --nodelist=$NODELIST ${PART:+--partition=$PART}"
for P in 2 4 8 16 32 31; do VERSAO=balanceada sbatch $OPT --ntasks=$P scripts/job.sh; done
