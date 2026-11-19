# T3 – Divisão e conquista com MPI

Ordenação de um vetor (decrescente, pior caso do Bubble Sort) por divisão e conquista com MPI,
medida no cluster Atlântica (LAD) em 2 nós exclusivos.

## Conteúdo

| Arquivo             | O que é                                                                                                                                                                                                    |
| ------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `original.c`        | Versão paralela SPMD da Figura 1: o pai envia as duas metades e espera; só as folhas ordenam.                                                                                                              |
| `balanceada.c`      | Versão balanceada: ao dividir, o processo envia uma metade ao ajudante `rank + 2^nível` e fica com a outra. Adaptada de Radenski (GPL, `mpi_mergesort.c`), com Bubble Sort nas folhas e vetor decrescente. |
| `sequencial.c`      | Referências sequenciais. Do professor, com o modo `-p P` acrescentado para percorrer a árvore da balanceada.                                                                                               |
| `sequencial_prof.c` | Cópia do `sequencial.c` original do professor, sem alterações.                                                                                                                                             |
| `scripts/`          | Scripts de compilação, calibração, séries de medição e conversão para CSV.                                                                                                                                 |
| `relatorio/`        | Relatório em LaTeX e PDF.                                                                                                                                                                                  |
| `resultados/`       | Saídas brutas do Slurm (criada pelos scripts).                                                                                                                                                             |
| `csv/`              | CSVs gerados a partir de `resultados/`.                                                                                                                                                                    |

## Modos do `sequencial`

```
./sequencial n            # T1: Bubble Sort do vetor inteiro
./sequencial n delta      # T1^delta: árvore da versão original em um só processo
./sequencial n -p P       # árvore da versão balanceada com P processos, em um só processo
```

## Como reproduzir

Rode tudo no Atlântica, na raiz do repositório.

1. **Compilar** (mesmas flags `-O2` para todas as versões):
   ```bash
   bash scripts/compilar.sh
   ```
2. **Calibrar N**, de modo que T1 fique entre 30 e 60 s:
   ```bash
   sbatch scripts/calibra.sh
   cat calibra-*.out        # tabela de resumo marca com "<== OK" os N válidos
   ```
3. **Configurar**: edite `scripts/config.sh` (`N`, `NODELIST` com os mesmos 2 nós em todas as séries, `PART` se necessário, `REPS=10`).
4. **Rodar as séries**, uma por vez (cada uma submete jobs via `sbatch`, 10 repetições por configuração):
   ```bash
   bash scripts/serie_seq.sh         # T1, T1^delta (P = 3..1023) e árvores da balanceada, em 1 nó
   bash scripts/serie_original.sh    # original, P = 1 3 7 15 31
   bash scripts/serie_balanceada.sh  # balanceada, P = 2 4 8 16 32 31
   bash scripts/serie_y0.sh          # original, P = 63..1023, OMPI_MCA_mpi_yield_when_idle=0
   bash scripts/serie_y1.sh          # original, P = 63..1023, OMPI_MCA_mpi_yield_when_idle=1
   ```
   Cada script aceita uma lista de P entre aspas para rodar só alguns, por exemplo `bash scripts/serie_y1.sh "511 1023"`.
   O número de jobs simultâneos na fila é limitado por `MAXJOBS` (padrão 2).
5. **Gerar os CSVs** para o Excel:
   ```bash
   bash scripts/para_csv.sh resultados csv
   ```
   Cria `paralelo.csv`, `sequencial.csv` e `por_processo.csv`, e informa quantas execuções deram `ERRO` na verificação.
6. **Relatório**: preencher `relatorio/relatorio.tex` com os números e compilar o PDF.

## Arquivos em `resultados/`

Cada job do Slurm gera um `.out` com o cabeçalho (`# data versao=... P=... N=... yield=...`), a distribuição dos processos entre os nós e as 10 repetições (`REP 1` a `REP 10`). `<jobid>` é o número do job no Slurm.

| Padrão do nome                 | Série                 | Conteúdo                                                                                                                      |
| ------------------------------ | --------------------- | ----------------------------------------------------------------------------------------------------------------------------- |
| `seq-<jobid>.out`              | `serie_seq.sh`        | `lscpu`, T1, T1^δ para P = 3 a 1023 e as árvores da balanceada (P = 2, 4, 8, 16, 31, 32). Um único arquivo, linhas `SEQ ...`. |
| `t3-<jobid>.out`               | `serie_original.sh`   | Versão original, 1 arquivo por P (1, 3, 7, 15, 31). O nome não traz o P, que está no cabeçalho (`P=`).                        |
| `balanceada-P<P>-<jobid>.out`  | `serie_balanceada.sh` | Versão balanceada, P = 2, 4, 8, 16, 32 e 31.                                                                                  |
| `original-y0-P<P>-<jobid>.out` | `serie_y0.sh`         | Original com P = 63 a 1023 e `yield_when_idle=0` (espera ativa).                                                              |
| `original-y1-P<P>-<jobid>.out` | `serie_y1.sh`         | Original com P = 63 a 1023 e `yield_when_idle=1` (cede o núcleo).                                                             |

Dentro dos arquivos paralelos, as linhas `RESULT` trazem o tempo e a verificação de cada repetição, e as linhas `STAT` trazem os dados por processo (rank, nível, tamanho, tempos de Bubble Sort, intercalação e espera). O `para_csv.sh` lê esses campos do conteúdo, não do nome do arquivo.

O `calibra.sh` é a exceção: grava `calibra-<jobid>.out` na raiz do repositório, fora de `resultados/`.

## Regras de medição seguidas

- Nós exclusivos (`--exclusive`) e sempre os mesmos 2 nós; `--overcommit` para P maior que as 32 threads de hardware.
- Tempo medido com `MPI_Wtime()` na raiz, da distribuição do vetor até a raiz ter o vetor ordenado. A inicialização fica de fora.
- Toda execução verifica se o vetor final está ordenado e imprime o resultado (`OK` ou `ERRO`).
- `delta = ceil(2n / (P+1))` na versão original, calculado em `scripts/job.sh`.
- `OMPI_MCA_mpi_yield_when_idle` definido explicitamente (0 e 1) nas séries de 63 a 1023 processos.
- Cada job registra a distribuição de processos entre os nós (`rank -> nó`) no cabeçalho da saída.
- 10 repetições por configuração; o relatório usa mediana, mínimo e máximo.

## Créditos

- `balanceada.c` parte do código de Radenski (licença GPL), `mpi_mergesort.c`.
- `sequencial.c` e o enunciado são do professor.
