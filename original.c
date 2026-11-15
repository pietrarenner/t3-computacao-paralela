/*
 * T3 - Versão ORIGINAL: divisão e conquista com MPI (ordenação de vetor).
 *
 * Árvore binária de processos, SPMD: pai, filhos e papel saem do rank.
 *   pai = (rank-1)/2   filho esquerdo = 2*rank+1   filho direito = 2*rank+2
 * Quem divide envia as duas metades e FICA PARADO esperando os filhos;
 * só as folhas conquistam (Bubble Sort). Use P = 2^k - 1 processos e
 * delta = ceil(2n/(P+1)) para que a árvore ocupe exatamente os P processos.
 *
 * Uso:   mpirun -np P ./original n delta
 * Compilação: mpicc -O2 -o original original.c
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG 1

/* Conquista: Bubble Sort com parada antecipada (igual ao sequencial.c). */
static void bubble_sort(int *v, int n)
{
    int trocou = 1;
    for (int fim = n - 1; fim > 0 && trocou; fim--) {
        trocou = 0;
        for (int i = 0; i < fim; i++) {
            if (v[i] > v[i + 1]) {
                int t = v[i]; v[i] = v[i + 1]; v[i + 1] = t;
                trocou = 1;
            }
        }
    }
}

/* Combinação: intercala v[0,n/2) e v[n/2,n) (igual ao sequencial.c). */
static void intercala(int *v, int n, int *aux)
{
    int meio = n / 2, i = 0, j = meio, k = 0;
    while (i < meio && j < n) aux[k++] = (v[i] <= v[j]) ? v[i++] : v[j++];
    while (i < meio) aux[k++] = v[i++];
    while (j < n)    aux[k++] = v[j++];
    memcpy(v, aux, (size_t)n * sizeof(int));
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int my_rank, P;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &P);

    if (argc != 3) {
        if (my_rank == 0) fprintf(stderr, "uso: %s n delta\n", argv[0]);
        MPI_Finalize();
        return 1;
    }
    int n = atoi(argv[1]);          /* tamanho do vetor (linha de comando) */
    int delta = atoi(argv[2]);      /* limiar de conquista (linha de comando) */

    /* Posição na árvore: tudo sai do rank. */
    int pai = (my_rank - 1) / 2;
    int filho_esq = 2 * my_rank + 1;
    int filho_dir = 2 * my_rank + 2;
    int nivel = 0;
    while ((1 << (nivel + 1)) - 1 <= my_rank) nivel++;   /* nivel = floor(log2(rank+1)) */

    int *vetor = NULL, tam_vetor = 0;
    double t_bubble = 0, t_merge = 0, t_espera = 0;      /* métricas por processo */
    double t0 = 0;

    if (my_rank == 0) {                                  /* raiz: gera o vetor */
        tam_vetor = n;
        vetor = malloc((size_t)n * sizeof(int));
        for (int i = 0; i < n; i++) vetor[i] = n - i;    /* decrescente; fora da medição */
    }
    MPI_Barrier(MPI_COMM_WORLD);                         /* todos prontos antes de medir */
    if (my_rank == 0) t0 = MPI_Wtime();                  /* início da região medida */

    if (my_rank != 0) {                                  /* recebo o vetor do pai */
        MPI_Status status;
        MPI_Probe(pai, TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &tam_vetor);     /* tamanho vem na mensagem */
        vetor = malloc((size_t)tam_vetor * sizeof(int));
        MPI_Recv(vetor, tam_vetor, MPI_INT, pai, TAG, MPI_COMM_WORLD, &status);
    }

    int folha = (tam_vetor <= delta);
    if (folha) {                                         /* conquisto */
        double a = MPI_Wtime();
        bubble_sort(vetor, tam_vetor);
        t_bubble = MPI_Wtime() - a;
    } else {                                             /* divido */
        if (filho_dir >= P) {
            fprintf(stderr, "rank %d: faltam processos (delta=%d, P=%d)\n", my_rank, delta, P);
            MPI_Abort(MPI_COMM_WORLD, 2);
        }
        int meio = tam_vetor / 2;
        /* as duas metades saem antes de qualquer espera: filhos trabalham juntos */
        MPI_Send(&vetor[0],    meio,             MPI_INT, filho_esq, TAG, MPI_COMM_WORLD);
        MPI_Send(&vetor[meio], tam_vetor - meio, MPI_INT, filho_dir, TAG, MPI_COMM_WORLD);
        double a = MPI_Wtime();
        MPI_Recv(&vetor[0],    meio,             MPI_INT, filho_esq, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&vetor[meio], tam_vetor - meio, MPI_INT, filho_dir, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        t_espera = MPI_Wtime() - a;
        int *aux = malloc((size_t)tam_vetor * sizeof(int));
        a = MPI_Wtime();
        intercala(vetor, tam_vetor, aux);
        t_merge = MPI_Wtime() - a;
        free(aux);
    }

    double tempo = 0;
    if (my_rank != 0) {
        MPI_Send(vetor, tam_vetor, MPI_INT, pai, TAG, MPI_COMM_WORLD);
    } else {
        tempo = MPI_Wtime() - t0;                        /* fim da região medida */
    }

    /* Estatísticas por processo (fora da região medida). */
    double mine[6] = { nivel, tam_vetor, folha, t_bubble, t_merge, t_espera };
    double *todos = (my_rank == 0) ? malloc((size_t)P * 6 * sizeof(double)) : NULL;
    MPI_Gather(mine, 6, MPI_DOUBLE, todos, 6, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {                                  /* verificação */
        long long erros = 0;
        for (int i = 0; i < n; i++) if (vetor[i] != i + 1) erros++;
        printf("RESULT versao=original n=%d delta=%d P=%d tempo=%.6f verificacao=%s\n",
               n, delta, P, tempo, erros == 0 ? "OK" : "ERRO");
        for (int r = 0; r < P; r++)
            printf("STAT rank=%d nivel=%d tam=%d folha=%d t_bubble=%.6f t_merge=%.6f t_espera=%.6f\n",
                   r, (int)todos[6*r], (int)todos[6*r+1], (int)todos[6*r+2],
                   todos[6*r+3], todos[6*r+4], todos[6*r+5]);
        free(todos);
    }
    free(vetor);
    MPI_Finalize();
    return 0;
}
