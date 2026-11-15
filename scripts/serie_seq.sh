#!/bin/bash
# Série: T1, T1delta (P=3..1023) e árvores da balanceada, em 1 nó (10 repetições por configuração, definidas em config.sh)
# Uso: bash scripts/serie_seq.sh   (da pasta do projeto, depois de editar scripts/config.sh)
cd "$(dirname "$0")/.."; source scripts/config.sh; : "${N:?edite N em scripts/config.sh}"
OPT="--nodes=2 --nodelist=$NODELIST ${PART:+--partition=$PART}"
sbatch --nodes=1 --exclusive ${PART:+--partition=$PART} scripts/seq.sh
