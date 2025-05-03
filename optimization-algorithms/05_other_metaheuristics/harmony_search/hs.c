#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>

#define max_n 3000
#define max_m 20
#define max_slot (max_n * 4)
#define w_words ((max_n + 63) / 64)

#define hs_size 25
#define hm_cr 0.95
#define par 0.4
#define bw 1
#define hs_iter 1000000
#define ls_iter 10000

static uint32_t rng_state;

static inline uint32_t fast_rand(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static inline double rand01(void) {
    return (double)fast_rand() / (double)UINT32_MAX;
}

int n, m, k;
int course_duration[max_n];
int room_capacity[max_m];

uint64_t conflict_bits[max_n][w_words];
uint64_t slot_bits[max_slot][w_words];

int rooms_count[max_n];
int rooms_list[max_n][max_m];

int slot_assignment[max_n];
int room_assignment[max_n];

typedef struct { 
    int cid, room, slot; 
} assign_rec;
static assign_rec last_assigns[max_n];
int last_count;

int room_usage[max_m][max_slot];

typedef struct {
    int s[max_n];
    int r[max_n];
    int cost;
} harmony;

static harmony hm[hs_size];
static harmony best_harmony;

static inline void clear_incremental(void) {
    for (int i = 0; i < last_count; i++) {
        assign_rec a = last_assigns[i];
        room_usage[a.room][a.slot] = -1;
        slot_bits[a.slot][a.cid>>6] &= ~(1ULL << (a.cid & 63));
    }
    last_count = 0;
}

static inline void record_assign(int cid, int room, int slot) {
    last_assigns[last_count++] = (assign_rec){cid, room, slot};
}

static inline bool can_assign_fast(int cid, int slot) {
    uint64_t *cb = conflict_bits[cid];
    uint64_t *sb = slot_bits[slot];
    for (int w = 0; w < w_words; w++) {
        if (cb[w] & sb[w]) return false;
    }
    return true;
}

static inline int compute_cost(const int *sv) {
    int mx = 0;
    for (int i = 0; i < n; i++) if (sv[i] > mx) mx = sv[i];
    return mx;
}

static inline bool assign_rooms(void) {
    clear_incremental();
    for (int i = 0; i < n; i++) {
        int slot0 = slot_assignment[i] - 1;
        bool placed = false;
        for (int ri = 0; ri < rooms_count[i]; ri++) {
            int rj = rooms_list[i][ri];
            if (room_usage[rj][slot0] != -1) continue;
            if (!can_assign_fast(i, slot0)) continue;
            room_assignment[i] = rj;
            room_usage[rj][slot0] = i;
            slot_bits[slot0][i>>6] |= 1ULL << (i & 63);
            record_assign(i, rj, slot0);
            placed = true;
            break;
        }
        if (!placed) return false;
    }
    return true;
}

static inline bool greedy_construct(int *order) {
    for (int r = 0; r < m; r++) for (int s = 0; s < best_harmony.cost; s++) room_usage[r][s] = -1;
    for (int s = 0; s < best_harmony.cost; s++) for (int w = 0; w < w_words; w++) slot_bits[s][w] = 0;
    last_count = 0;
    for (int idx = 0; idx < n; idx++) {
        int cid = order[idx];
        bool placed = false;
        for (int ri = 0; ri < rooms_count[cid] && !placed; ri++) {
            int rj = rooms_list[cid][ri];
            for (int s = 0; s < best_harmony.cost; s++) {
                if (room_usage[rj][s] != -1) continue;
                if (!can_assign_fast(cid, s)) continue;
                slot_assignment[cid] = s + 1;
                room_assignment[cid] = rj;
                room_usage[rj][s] = cid;
                slot_bits[s][cid>>6] |= 1ULL << (cid & 63);
                record_assign(cid, rj, s);
                placed = true;
                break;
            }
        }
        if (!placed) return false;
    }
    return true;
}

static inline void random_shuffle_order(int *a, int len) {
    for (int i = len - 1; i > 0; i--) {
        int j = fast_rand() % (i + 1);
        int t = a[i]; a[i] = a[j]; a[j] = t;
    }
}

static inline void harmony_search(void) {
    int order[max_n];
    for (int i = 0; i < n; i++) order[i] = i;
    random_shuffle_order(order, n);
    best_harmony.cost = max_slot;
    greedy_construct(order);
    best_harmony.cost = compute_cost(slot_assignment);
    for (int i = 0; i < n; i++) {
        best_harmony.s[i] = slot_assignment[i];
        best_harmony.r[i] = room_assignment[i];
    }
    int h = 0;
    hm[h++] = best_harmony;
    while (h < hs_size) {
        random_shuffle_order(order, n);
        if (!greedy_construct(order)) continue;
        harmony new_h;
        new_h.cost = compute_cost(slot_assignment);
        for (int i = 0; i < n; i++) {
            new_h.s[i] = slot_assignment[i];
            new_h.r[i] = room_assignment[i];
        }
        hm[h++] = new_h;
        if (new_h.cost < best_harmony.cost) best_harmony = new_h;
    }
    for (int iter = 0; iter < hs_iter; iter++) {
        for (int i = 0; i < n; i++) {
            double p = rand01();
            int si;
            if (p < hm_cr) {
                int rh = fast_rand() % hs_size;
                si = hm[rh].s[i];
                if (rand01() < par) {
                    int adj = ((int)(fast_rand() % (2*bw+1))) - bw;
                    si += adj;
                    if (si < 1) si = 1;
                    if (si > best_harmony.cost) si = best_harmony.cost;
                }
            } else {
                si = (fast_rand() % best_harmony.cost) + 1;
            }
            slot_assignment[i] = si;
        }
        if (!assign_rooms()) continue;
        int cost = compute_cost(slot_assignment);
        int w = 0;
        for (int j = 1; j < hs_size; j++) if (hm[j].cost > hm[w].cost) w = j;
        if (cost < hm[w].cost) {
            hm[w].cost = cost;
            for (int i = 0; i < n; i++) {
                hm[w].s[i] = slot_assignment[i];
                hm[w].r[i] = room_assignment[i];
            }
            if (cost < best_harmony.cost) best_harmony = hm[w];
        }
    }
}

static inline void build_usage_from_best(void) {
    for (int r = 0; r < m; r++) for (int s = 0; s < best_harmony.cost; s++) room_usage[r][s] = -1;
    for (int s = 0; s < best_harmony.cost; s++) for (int w = 0; w < w_words; w++) slot_bits[s][w] = 0;
    last_count = 0;
    for (int i = 0; i < n; i++) {
        int s0 = best_harmony.s[i] - 1;
        int rj = best_harmony.r[i];
        room_usage[rj][s0] = i;
        slot_bits[s0][i>>6] |= 1ULL << (i & 63);
    }
}

static inline void local_search(void) {
    build_usage_from_best();
    bool improved = true;
    int iter = 0;
    while (improved && iter++ < ls_iter) {
        improved = false;
        for (int i = 0; i < n && !improved; i++) {
            int cur_s = best_harmony.s[i];
            int old_r = best_harmony.r[i];
            int old_slot = cur_s - 1;
            room_usage[old_r][old_slot] = -1;
            slot_bits[old_slot][i>>6] &= ~(1ULL << (i & 63));
            for (int ns = 1; ns < cur_s && !improved; ns++) {
                for (int ri = 0; ri < rooms_count[i]; ri++) {
                    int rj = rooms_list[i][ri];
                    if (room_usage[rj][ns-1] != -1) continue;
                    if (!can_assign_fast(i, ns-1)) continue;
                    best_harmony.s[i] = ns;
                    best_harmony.r[i] = rj;
                    room_usage[rj][ns-1] = i;
                    slot_bits[ns-1][i>>6] |= 1ULL << (i & 63);
                    improved = true;
                    break;
                }
            }
            if (!improved) {
                room_usage[old_r][old_slot] = i;
                slot_bits[old_slot][i>>6] |= 1ULL << (i & 63);
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

    harmony_search();
    local_search();
    for (int i = 0; i < n; i++) {
        printf("%d %d %d\n", i+1, best_harmony.s[i], best_harmony.r[i]+1);
    }
    return 0;
}
