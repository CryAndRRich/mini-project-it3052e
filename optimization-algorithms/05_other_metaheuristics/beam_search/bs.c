#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#define max_n 3000
#define max_m 20
#define max_slot (max_n * 4)
#define w_words ((max_n + 63) / 64)

#define beam_width 5
#define num_permutations 100

static uint32_t rng_state;

static inline uint32_t fast_rand(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static inline void random_shuffle(int *arr, int len) {
    for (int i = len - 1; i > 0; i--) {
        int j = fast_rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int n, m, k;
int course_duration[max_n];
int room_capacity[max_m];

uint64_t conflict_bits[max_n][w_words];

int rooms_count[max_n];
int rooms_list[max_n][max_m];
int order[max_n];

typedef struct {
    int assigned;
    int s[max_n];
    int r[max_n];
    int cost;
} state;

int best_slot_assignment[max_n];
int best_room_assignment[max_n];
int best_cost = max_slot;

int cmp_state(const void *a, const void *b) {
    return ((state*)a)->cost - ((state*)b)->cost;
}

static inline bool can_place(const state *st, int idx, int cid, int slot0, int rj) {
    for (int t = 0; t < idx; t++) {
        int j = order[t];
        if (st->s[j] - 1 == slot0) {
            if (st->r[j] == rj) return false;
            int w = j >> 6;
            if (conflict_bits[cid][w] & (1ULL << (j & 63))) return false;
        }
    }
    return true;
}

static inline void beam_search(void) {
    state *beam = malloc(sizeof(state) * beam_width);
    state *next = malloc(sizeof(state) * beam_width * max_slot);
    int beam_sz = 1;
    beam[0].assigned = 0;
    beam[0].cost = 0;

    for (int idx = 0; idx < n; idx++) {
        int cid = order[idx];
        int next_sz = 0;
        for (int b = 0; b < beam_sz; b++) {
            state *st = &beam[b];
            int slot_limit = st->cost + 1;
            if (slot_limit > max_slot) slot_limit = max_slot;
            for (int slot = 1; slot <= slot_limit; slot++) {
                int slot0 = slot - 1;
                for (int ri = 0; ri < rooms_count[cid]; ri++) {
                    int rj = rooms_list[cid][ri];
                    if (can_place(st, idx, cid, slot0, rj)) {
                        state ns = *st;
                        ns.assigned = idx + 1;
                        ns.s[cid] = slot;
                        ns.r[cid] = rj;
                        ns.cost = slot > ns.cost ? slot : ns.cost;
                        next[next_sz++] = ns;
                        if (next_sz >= beam_width * max_slot) break;
                    }
                }
                if (next_sz >= beam_width * max_slot) break;
            }
        }
        if (next_sz > beam_width) {
            qsort(next, next_sz, sizeof(state), cmp_state);
            next_sz = beam_width;
        }
        beam_sz = next_sz;
        for (int i = 0; i < beam_sz; i++) beam[i] = next[i];
    }
    qsort(beam, beam_sz, sizeof(state), cmp_state);
    state *best = &beam[0];
    if (best->cost < best_cost) {
        best_cost = best->cost;
        for (int i = 0; i < n; i++) {
            best_slot_assignment[i] = best->s[i];
            best_room_assignment[i] = best->r[i];
        }
    }
    free(beam);
    free(next);
}

static inline void local_search(void) {
    bool improved = true;
    while (improved) {
        improved = false;
        for (int i = 0; i < n && !improved; i++) {
            int cur_s = best_slot_assignment[i];
            for (int ns = 1; ns < cur_s && !improved; ns++) {
                for (int ri = 0; ri < rooms_count[i]; ri++) {
                    int rj = rooms_list[i][ri];
                    bool ok = true;
                    for (int j = 0; j < n; j++) {
                        if (j == i) continue;
                        if (best_slot_assignment[j] == ns) {
                            if (best_room_assignment[j] == rj) { ok = false; break; }
                            int w = j >> 6;
                            if (conflict_bits[i][w] & (1ULL << (j & 63))) { ok = false; break; }
                        }
                    }
                    if (!ok) continue;
                    best_slot_assignment[i] = ns;
                    best_room_assignment[i] = rj;
                    improved = true;
                    break;
                }
            }
        }
    }
}

void input_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen"); exit(EXIT_FAILURE); }
    fscanf(f, "%d %d", &n, &m);
    for (int i = 0; i < n; i++) fscanf(f, "%d", &course_duration[i]);
    for (int j = 0; j < m; j++) fscanf(f, "%d", &room_capacity[j]);
    fscanf(f, "%d", &k);
    for (int i = 0; i < n; i++)
        for (int w = 0; w < w_words; w++)
            conflict_bits[i][w] = 0ULL;
    for (int e = 0; e < k; e++) {
        int u, v;
        fscanf(f, "%d %d", &u, &v);
        u--; v--;
        conflict_bits[u][v>>6] |= 1ULL << (v & 63);
        conflict_bits[v][u>>6] |= 1ULL << (u & 63);
    }
    for (int i = 0; i < n; i++) {
        int cnt = 0;
        for (int j = 0; j < m; j++) {
            if (room_capacity[j] >= course_duration[i]) {
                rooms_list[i][cnt++] = j;
            }
        }
        rooms_count[i] = cnt;
    }
    fclose(f);
}

void input_manual(void) {
    scanf("%d %d", &n, &m);
    for (int i = 0; i < n; i++) scanf("%d", &course_duration[i]);
    for (int j = 0; j < m; j++) scanf("%d", &room_capacity[j]);
    scanf("%d", &k);
    for (int i = 0; i < n; i++)
        for (int w = 0; w < w_words; w++)
            conflict_bits[i][w] = 0ULL;
    for (int e = 0; e < k; e++) {
        int u, v;
        scanf("%d %d", &u, &v);
        u--; v--;
        conflict_bits[u][v>>6] |= 1ULL << (v & 63);
        conflict_bits[v][u>>6] |= 1ULL << (u & 63);
    }
    for (int i = 0; i < n; i++) {
        int cnt = 0;
        for (int j = 0; j < m; j++) {
            if (room_capacity[j] >= course_duration[i]) {
                rooms_list[i][cnt++] = j;
            }
        }
        rooms_count[i] = cnt;
    }
}

int main(int argc, char **argv) {
    rng_state = (uint32_t)time(NULL);
    if (argc >= 2) {
        input_file(argv[1]);
    } else {
        input_manual();
    }

    for (int p = 0; p < num_permutations; p++) {
        for (int i = 0; i < n; i++) order[i] = i;
        random_shuffle(order, n);
        beam_search();
    }
    local_search();

    for (int i = 0; i < n; i++) {
        printf("%d %d %d\n", i+1, best_slot_assignment[i], best_room_assignment[i] + 1);
    }
    return 0;
}
