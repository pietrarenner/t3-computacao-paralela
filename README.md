# T3 – Divisão e conquista com MPI
Código: original.c, balanceada.c (adaptada de Radenski, GPL), sequencial.c (do professor + modo -p P).
1. `bash scripts/compilar.sh`
2. `sbatch scripts/calibra.sh` -> escolha N; edite `scripts/config.sh` (N, NODELIST, PART)
3. Uma série por vez (10 repetições cada): `bash scripts/serie_seq.sh`, `serie_original.sh`, `serie_balanceada.sh`, `serie_y0.sh`, `serie_y1.sh`
4. `bash scripts/para_csv.sh resultados csv` -> CSVs para o Excel
5. Preencher relatorio/relatorio.tex
