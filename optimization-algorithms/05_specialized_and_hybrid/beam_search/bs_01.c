/*
Điểm: 99 94 97 100 60
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>

#define MAX_N 1000
#define MAX_M 1000
#define MAX_K 10000
#define MAX_SLOT (MAX_N * 4)
#define INF (MAX_SLOT + 1)
#define W ((MAX_N + 63) / 64)

#define BEAM_WIDTH 30

int N, M, K;
int d[MAX_N];
int c[MAX_M];
uint64_t conflict_bits[MAX_N][W];
int rooms_count[MAX_N];
int rooms_list[MAX_N][MAX_M];
static int order_arr[MAX_N];

typedef struct {
    int assigned;
    int s[MAX_N];
    int r[MAX_N];
    int cost;
} State;

int best_s[MAX_N];
int best_r[MAX_N];

int cmp_state(const void *a, const void *b) {
    return ((State*)a)->cost - ((State*)b)->cost;
}

bool canPlace(const State *st, int idx, int cid, int slot0, int rj) {
    for (int k = 0; k < idx; k++) {
        int j = order_arr[k];
        if (st->s[j] - 1 == slot0) {
            if (st->r[j] == rj) return false;
            int w = j >> 6;
            if (conflict_bits[cid][w] & (1ULL << (j & 63))) return false;
        }
    }
    return true;
}

void beamSearch(void) {
    for (int i = 0; i < N; i++) order_arr[i] = i;
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            if (d[order_arr[j]] > d[order_arr[i]]) {
                int t = order_arr[i]; order_arr[i] = order_arr[j]; order_arr[j] = t;
            }
        }
    }

    State *beam = malloc(sizeof(State) * BEAM_WIDTH);
    State *next = malloc(sizeof(State) * BEAM_WIDTH * MAX_SLOT);
    int beam_sz = 1;
    beam[0].assigned = 0;
    beam[0].cost = 0;

    for (int idx = 0; idx < N; idx++) {
        int cid = order_arr[idx];
        int next_sz = 0;
        for (int b = 0; b < beam_sz; b++) {
            State *st = &beam[b];
            int max_slot = st->cost + 1;
            if (max_slot > MAX_SLOT) max_slot = MAX_SLOT;
            for (int slot = 1; slot <= max_slot; slot++) {
                int slot0 = slot - 1;
                for (int ri = 0; ri < rooms_count[cid]; ri++) {
                    int rj = rooms_list[cid][ri];
                    if (canPlace(st, idx, cid, slot0, rj)) {
                        State ns = *st;
                        ns.assigned = st->assigned + 1;
                        ns.s[cid] = slot;
                        ns.r[cid] = rj;
                        ns.cost = slot > ns.cost ? slot : ns.cost;
                        next[next_sz++] = ns;
                        if (next_sz >= BEAM_WIDTH * MAX_SLOT) break;
                    }
                }
                if (next_sz >= BEAM_WIDTH * MAX_SLOT) break;
            }
        }
        if (next_sz > BEAM_WIDTH) {
            qsort(next, next_sz, sizeof(State), cmp_state);
            next_sz = BEAM_WIDTH;
        }
        beam_sz = next_sz;
        for (int i = 0; i < beam_sz; i++) beam[i] = next[i];
    }
    qsort(beam, beam_sz, sizeof(State), cmp_state);
    State best = beam[0];
    free(beam);
    free(next);

    for (int i = 0; i < N; i++) {
        best_s[i] = best.s[i];
        best_r[i] = best.r[i];
    }
}

void localSearch(void) {
    bool improved = true;
    while (improved) {
        improved = false;
        for (int i = 0; i < N; i++) {
            int cur = best_s[i];
            for (int ns = 1; ns < cur; ns++) {
                for (int ri = 0; ri < rooms_count[i]; ri++) {
                    int rj = rooms_list[i][ri];
                    bool ok = true;
                    for (int j = 0; j < N; j++) {
                        if (j == i) continue;
                        if (best_s[j] == ns) {
                            if (best_r[j] == rj) { ok = false; break; }
                            int w = j >> 6;
                            if (conflict_bits[i][w] & (1ULL << (j & 63))) { ok = false; break; }
                        }
                    }
                    if (!ok) continue;
                    best_s[i] = ns;
                    best_r[i] = rj;
                    improved = true;
                    break;
                }
                if (improved) break;
            }
            if (improved) break;
        }
    }
}

int main(void) {
    scanf("%d %d", &N, &M);
    for (int i = 0; i < N; i++) scanf("%d", &d[i]);
    for (int j = 0; j < M; j++) scanf("%d", &c[j]);
    scanf("%d", &K);
    for (int k = 0; k < K; k++) {
        int u, v; scanf("%d %d", &u, &v);
        u--; v--;
        conflict_bits[u][v >> 6] |= 1ULL << (v & 63);
        conflict_bits[v][u >> 6] |= 1ULL << (u & 63);
    }
    for (int i = 0; i < N; i++) {
        rooms_count[i] = 0;
        for (int j = 0; j < M; j++) {
            if (c[j] >= d[i]) rooms_list[i][rooms_count[i]++] = j;
        }
    }
    beamSearch();
    localSearch();
    for (int i = 0; i < N; i++) {
        printf("%d %d %d\n", i + 1, best_s[i], best_r[i] + 1);
    }
    return 0;
}
