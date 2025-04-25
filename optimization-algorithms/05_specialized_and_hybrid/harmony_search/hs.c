/*
Exam scheduling using Harmony Search + Local Search refinement
- Seed HM via Greedy
- Incremental clear of assignments
- static inline and optimized bit-check
- Local Search to polish best solution
- Fast rand implementation

Điểm: 98 93 97 100 60
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

#define HS_SIZE 25
#define HMCR 0.95
#define PAR 0.4
#define BW 1
#define HS_ITER 1000000
#define LS_ITER 10000

int N, M, K;
int d[MAX_N];
int c[MAX_M];
uint64_t conflict_bits[MAX_N][W];

int s[MAX_N];
int r_room[MAX_N];

int room_usage[MAX_M][MAX_SLOT];
uint64_t slot_bits[MAX_SLOT][W];

int rooms_count[MAX_N];
int rooms_list[MAX_N][MAX_M];

typedef struct {
    int s[MAX_N];
    int r[MAX_N];
    int cost;
} Harmony;

Harmony bestHarmony;
Harmony HM[HS_SIZE];

static uint32_t rng_state;

static inline uint32_t fast_rand(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static inline double rand01(void) {
    return (double)fast_rand() / UINT32_MAX;
}

typedef struct { int cid, room, slot; } Assign;
static Assign last_assigns[MAX_N];
static int last_count;

static inline void clearIncremental(void) {
    for (int i = 0; i < last_count; i++) {
        Assign a = last_assigns[i];
        room_usage[a.room][a.slot] = -1;
        slot_bits[a.slot][a.cid >> 6] &= ~(1ULL << (a.cid & 63));
    }
    last_count = 0;
}

static inline void recordAssign(int cid, int room, int slot0) {
    last_assigns[last_count++] = (Assign){ cid, room, slot0 };
}

static inline bool canAssignFast(int cid, int slot0) {
    uint64_t *cb = conflict_bits[cid];
    uint64_t *sb = slot_bits[slot0];
    for (int w = 0; w < W; w++) {
        if (cb[w] & sb[w]) return false;
    }
    return true;
}

static inline int computeCost(const int *sv) {
    int mx = 0;
    for (int i = 0; i < N; i++) {
        if (sv[i] > mx) mx = sv[i];
    }
    return mx;
}

bool assignRooms(void) {
    clearIncremental();
    for (int i = 0; i < N; i++) {
        int slot0 = s[i] - 1;
        bool ok = false;
        for (int ri = 0; ri < rooms_count[i]; ri++) {
            int rj = rooms_list[i][ri];
            if (room_usage[rj][slot0] != -1) continue;
            if (!canAssignFast(i, slot0)) continue;
            r_room[i] = rj;
            room_usage[rj][slot0] = i;
            slot_bits[slot0][i >> 6] |= 1ULL << (i & 63);
            recordAssign(i, rj, slot0);
            ok = true;
            break;
        }
        if (!ok) return false;
    }
    return true;
}

bool greedyAssignment(int order[]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < bestHarmony.cost; j++) {
            room_usage[i][j] = -1;
        }
    }
    for (int k = 0; k < bestHarmony.cost; k++) {
        for (int w = 0; w < W; w++) {
            slot_bits[k][w] = 0;
        }
    }
    last_count = 0;

    for (int idx = 0; idx < N; idx++) {
        int cid = order[idx];
        bool placed = false;
        for (int ri = 0; ri < rooms_count[cid]; ri++) {
            int rj = rooms_list[cid][ri];
            for (int slot0 = 0; slot0 < bestHarmony.cost; slot0++) {
                if (room_usage[rj][slot0] != -1) continue;
                if (!canAssignFast(cid, slot0)) continue;
                s[cid] = slot0 + 1;
                r_room[cid] = rj;
                room_usage[rj][slot0] = cid;
                slot_bits[slot0][cid >> 6] |= 1ULL << (cid & 63);
                recordAssign(cid, rj, slot0);
                placed = true;
                break;
            }
            if (placed) break;
        }
        if (!placed) return false;
    }
    return true;
}

void random_shuffle(int *a, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = fast_rand() % (i + 1);
        int t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
}

void harmonySearch(void) {
    int order[MAX_N];
    for (int i = 0; i < N; i++) order[i] = i;
    random_shuffle(order, N);
    bestHarmony.cost = MAX_SLOT;
    greedyAssignment(order);
    bestHarmony.cost = computeCost(s);
    for (int i = 0; i < N; i++) {
        bestHarmony.s[i] = s[i];
        bestHarmony.r[i] = r_room[i];
    }
    int h = 0;
    HM[h++] = bestHarmony;
    while (h < HS_SIZE) {
        random_shuffle(order, N);
        if (!greedyAssignment(order)) continue;
        int cost = computeCost(s);
        HM[h].cost = cost;
        for (int i = 0; i < N; i++) {
            HM[h].s[i] = s[i];
            HM[h].r[i] = r_room[i];
        }
        if (cost < bestHarmony.cost) bestHarmony = HM[h];
        h++;
    }
    for (int iter = 0; iter < HS_ITER; iter++) {
        for (int i = 0; i < N; i++) {
            double p = rand01();
            int si;
            if (p < HMCR) {
                int rh = fast_rand() % HS_SIZE;
                si = HM[rh].s[i];
                if (rand01() < PAR) {
                    int adj = (fast_rand() % (2 * BW + 1)) - BW;
                    si += adj;
                    if (si < 1) si = 1;
                    if (si > bestHarmony.cost) si = bestHarmony.cost;
                }
            } else {
                si = (fast_rand() % bestHarmony.cost) + 1;
            }
            s[i] = si;
        }
        if (!assignRooms()) continue;
        int cost = computeCost(s);
        int w = 0;
        for (int j = 1; j < HS_SIZE; j++) {
            if (HM[j].cost > HM[w].cost) w = j;
        }
        if (cost < HM[w].cost) {
            HM[w].cost = cost;
            for (int i = 0; i < N; i++) {
                HM[w].s[i] = s[i];
                HM[w].r[i] = r_room[i];
            }
            if (cost < bestHarmony.cost) bestHarmony = HM[w];
        }
    }
}

void buildUsageFromBest(void) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < bestHarmony.cost; j++) {
            room_usage[i][j] = -1;
        }
    }
    for (int k = 0; k < bestHarmony.cost; k++) {
        for (int w = 0; w < W; w++) {
            slot_bits[k][w] = 0;
        }
    }
    last_count = 0;
    for (int i = 0; i < N; i++) {
        s[i] = bestHarmony.s[i];
        r_room[i] = bestHarmony.r[i];
        int slot0 = s[i] - 1;
        int rj = r_room[i];
        room_usage[rj][slot0] = i;
        slot_bits[slot0][i >> 6] |= 1ULL << (i & 63);
    }
}

void localSearchRefine(void) {
    buildUsageFromBest();
    bool improved = true;
    int iter = 0;
    while (improved && iter < LS_ITER) {
        improved = false;
        iter++;
        for (int i = 0; i < N; i++) {
            int cur = s[i];
            for (int ns = 1; ns < cur; ns++) {
                int old_r = r_room[i];
                int old_slot = cur - 1;
                room_usage[old_r][old_slot] = -1;
                slot_bits[old_slot][i >> 6] &= ~(1ULL << (i & 63));
                bool ok = false;
                for (int ri = 0; ri < rooms_count[i]; ri++) {
                    int rj = rooms_list[i][ri];
                    if (room_usage[rj][ns - 1] != -1) continue;
                    if (!canAssignFast(i, ns - 1)) continue;
                    s[i] = ns;
                    r_room[i] = rj;
                    room_usage[rj][ns - 1] = i;
                    slot_bits[ns - 1][i >> 6] |= 1ULL << (i & 63);
                    ok = true;
                    break;
                }
                if (ok) {
                    improved = true;
                    break;
                }
                room_usage[old_r][old_slot] = i;
                slot_bits[old_slot][i >> 6] |= 1ULL << (i & 63);
                s[i] = cur;
                r_room[i] = old_r;
            }
        }
    }
    int newCost = computeCost(s);
    if (newCost < bestHarmony.cost) {
        bestHarmony.cost = newCost;
        for (int i = 0; i < N; i++) {
            bestHarmony.s[i] = s[i];
            bestHarmony.r[i] = r_room[i];
        }
    }
}

int main(void) {
    rng_state = (uint32_t)time(NULL);
    scanf("%d %d", &N, &M);
    for (int i = 0; i < N; i++) scanf("%d", &d[i]);
    for (int j = 0; j < M; j++) scanf("%d", &c[j]);
    scanf("%d", &K);
    for (int k = 0; k < K; k++) {
        int u, v;
        scanf("%d %d", &u, &v);
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
    harmonySearch();
    localSearchRefine();
    for (int i = 0; i < N; i++) {
        printf("%d %d %d\n", i + 1, bestHarmony.s[i], bestHarmony.r[i] + 1);
    }
    return 0;
}
