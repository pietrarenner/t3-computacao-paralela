/* T3 - Versao BALANCEADA (variante MPI_Send): ao dividir, o processo envia uma metade a um
   ajudante e FICA com a outra, de modo que os internos tambem conquistam.

   Adaptado de "MPI recursive merge sort" (mpi_mergesort.c)
   Copyright (C) 2011  Atanas Radenski

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of
 the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public
 License along with this program; if not, write to the Free
 Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA  02110-1301, USA.

   Alteracoes para o T3 (versao balanceada):
   - folhas ordenam com Bubble Sort (no lugar de merge/insertion sort);
   - vetor inicial decrescente (no lugar de aleatorio);
   - tempo com MPI_Wtime(), na mesma regiao da versao original;
   - pai calculado pelo rank (sem MPI_ANY_SOURCE) e 2^nivel por deslocamento
     de bit (no lugar de pow());
   - envio da 2a metade com MPI_Send bloqueante (no lugar de MPI_Isend +
     MPI_Request_free, como em balanceada.c): com mensagens grandes entre nos, o MPI_Isend so avanca
     quando o remetente volta ao MPI, e as folhas do outro no comecavam depois
     do Bubble Sort do pai (tempo medido = 2x o Bubble Sort da maior folha);
   - estatisticas por processo e verificacao contra 1..n.

   Esquema: no nivel L, o processo r tem ajudante r + 2^L. O ajudante r' recebe
   o pedaco, comeca no nivel bits(r') e seu pai e r' sem o bit mais alto.
   Funciona para qualquer P (com P = 2^k todos terminam com n/P elementos).

   Uso:   mpirun -np P ./balanceada n
   Compilacao: mpicc -O2 -o balanceada balanceada.c
*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG 1

/* estatísticas deste processo */
static int    g_tam_folha = 0, g_divisoes = 0;
static double g_t_bubble = 0, g_t_merge = 0, g_t_espera = 0;

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

static void intercala(int *v, int n, int *aux)
{
    int meio = n / 2, i = 0, j = meio, k = 0;
    while (i < meio && j < n) aux[k++] = (v[i] <= v[j]) ? v[i++] : v[j++];
    while (i < meio) aux[k++] = v[i++];
    while (j < n)    aux[k++] = v[j++];
    memcpy(v, aux, (size_t)n * sizeof(int));
}

/* Número de bits de r = nível em que o processo r entra na árvore (r=0 -> 0). */
static int nivel_de_entrada(int r)
{
    int nivel = 0;
    while ((1 << nivel) <= r) nivel++;
    return nivel;
}

/*
 * Ordena v[0,n). Enquanto houver ajudante (rank + 2^nivel <= max_rank),
 * manda a 2ª metade a ele, ordena a 1ª recursivamente no
 * próximo nível, recebe a 2ª ordenada e intercala. Sem ajudante: conquista.
 */
static void ordena_paralelo(int *v, int n, int *aux, int nivel, int my_rank, int max_rank)
{
    int ajudante = my_rank + (1 << nivel);
    if (ajudante > max_rank) {                     /* conquista */
        double a = MPI_Wtime();
        bubble_sort(v, n);
        g_t_bubble += MPI_Wtime() - a;
        g_tam_folha = n;
        return;
    }
    g_divisoes++;
    /* Envio bloqueante da 2a metade. O ajudante ja espera em MPI_Probe/MPI_Recv,
       entao o MPI_Send termina logo e as duas metades sao ordenadas ao mesmo
       tempo; quando volta, o buffer v+n/2 ja pode ser lido de novo. */
    MPI_Send(v + n / 2, n - n / 2, MPI_INT, ajudante, TAG, MPI_COMM_WORLD); // chamada bloqueante: envia metade direita ao ajudante (rank + 2^nivel) e espera entrega
    // como ajudante ja esta esperando, o MPI_Send termina em milissegundos e as duas metades sao ordenadas ao mesmo tempo
    ordena_paralelo(v, n / 2, aux, nivel + 1, my_rank, max_rank);
    double a = MPI_Wtime();
    MPI_Recv(v + n / 2, n - n / 2, MPI_INT, ajudante, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    g_t_espera += MPI_Wtime() - a;
    a = MPI_Wtime();
    intercala(v, n, aux);
    g_t_merge += MPI_Wtime() - a;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int my_rank, P;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &P);
    if (argc != 2) {
        if (my_rank == 0) fprintf(stderr, "uso: %s n\n", argv[0]);
        MPI_Finalize();
        return 1;
    }
    int n = atoi(argv[1]);
    int max_rank = P - 1;
    int *v = NULL, *aux = NULL, tam = n, pai = -1, nivel = 0;
    double t0 = 0, tempo = 0;

    if (my_rank == 0) {
        v = malloc((size_t)n * sizeof(int));
        for (int i = 0; i < n; i++) v[i] = n - i;       /* decrescente */
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (my_rank == 0) {
        t0 = MPI_Wtime();
        aux = malloc((size_t)n * sizeof(int));
        ordena_paralelo(v, n, aux, 0, 0, max_rank);
        tempo = MPI_Wtime() - t0;
    } else {
        nivel = nivel_de_entrada(my_rank);
        pai = my_rank - (1 << (nivel - 1));              /* limpa o bit mais alto */
        MPI_Status st;
        MPI_Probe(pai, TAG, MPI_COMM_WORLD, &st);
        MPI_Get_count(&st, MPI_INT, &tam);
        v = malloc((size_t)tam * sizeof(int));
        aux = malloc((size_t)tam * sizeof(int));
        MPI_Recv(v, tam, MPI_INT, pai, TAG, MPI_COMM_WORLD, &st);
        ordena_paralelo(v, tam, aux, nivel, my_rank, max_rank);
        MPI_Send(v, tam, MPI_INT, pai, TAG, MPI_COMM_WORLD);
    }

    double mine[6] = { nivel, tam, g_tam_folha, g_t_bubble, g_t_merge, g_t_espera };
    double *todos = (my_rank == 0) ? malloc((size_t)P * 6 * sizeof(double)) : NULL;
    MPI_Gather(mine, 6, MPI_DOUBLE, todos, 6, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        long long erros = 0;
        for (int i = 0; i < n; i++) if (v[i] != i + 1) erros++;
        printf("RESULT versao=balanceada_send n=%d delta=0 P=%d tempo=%.6f verificacao=%s\n",
               n, P, tempo, erros == 0 ? "OK" : "ERRO");
        for (int r = 0; r < P; r++)
            printf("STAT rank=%d nivel=%d tam=%d folha=%d t_bubble=%.6f t_merge=%.6f t_espera=%.6f\n",
                   r, (int)todos[6*r], (int)todos[6*r+1], (int)todos[6*r+2],
                   todos[6*r+3], todos[6*r+4], todos[6*r+5]);
        free(todos);
    }
    free(v); free(aux);
    MPI_Finalize();
    return 0;
}
