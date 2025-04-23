/*
Iterated Local Search

Điểm: 96 93 96 100 60
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>

#define MAX_N 1000
#define MAX_M 1000
#define MAX_SLOT (MAX_N * 4)
#define INF (MAX_SLOT + 1)
#define ILS_ITERS 10000
#define PERTURB_K 5
#define RANDOM_TRIALS 100
#define W ((MAX_N + 63) / 64)

int N, M, K;
int d[MAX_N];        
int c[MAX_M];       
uint64_t conflict_bits[MAX_N][W];

int s[MAX_N], r[MAX_N];            
int currS[MAX_N], currR[MAX_N];  
int bestS[MAX_N], bestR[MAX_N];   
int bestCost = INF;

static int room_ts[MAX_M][MAX_SLOT];
static int slot_ts[MAX_SLOT][W];
static int epoch = 1;
static uint64_t slot_bits[MAX_SLOT][W];
static int count_slot[MAX_SLOT+1];
static int curMaxSlot;

static inline void clearUsage() {
    epoch++;
    curMaxSlot = 0;
    for (int i = 1; i <= MAX_SLOT; i++) count_slot[i] = 0;
}

static inline bool isRoomFree(int rm, int sl) {
    return room_ts[rm][sl] != epoch;
}

static inline bool canAssign(int cid, int slot) {
    uint64_t *cb = conflict_bits[cid];
    int idx = slot - 1;
    for (int w = 0; w < W; w++) {
        uint64_t sb = (slot_ts[idx][w] == epoch ? slot_bits[idx][w] : 0ULL);
        if (cb[w] & sb) return false;
    }
    return true;
}

static inline void applyAssign(int cid, int rm, int sl) {
    room_ts[rm][sl] = epoch;
    slot_ts[sl][cid>>6] = epoch;
    slot_bits[sl][cid>>6] |= 1ULL << (cid & 63);
    int idx = sl + 1;
    count_slot[idx]++;
    if (idx > curMaxSlot) curMaxSlot = idx;
}

static inline void removeAssign(int cid, int rm, int sl) {
    room_ts[rm][sl] = -epoch;
    if (slot_ts[sl][cid>>6] == epoch) slot_bits[sl][cid>>6] &= ~(1ULL << (cid & 63));
    int idx = sl + 1;
    if (--count_slot[idx] == 0 && idx == curMaxSlot) {
        while (curMaxSlot > 0 && count_slot[curMaxSlot] == 0) curMaxSlot--;
    }
}

static inline void buildUsageFromSolution() {
    clearUsage();
    for (int i = 0; i < N; i++) {
        if (s[i] > 0 && r[i] > 0) {
            int sl = s[i] - 1;
            int rm = r[i] - 1;
            room_ts[rm][sl] = epoch;
            slot_ts[sl][i>>6] = epoch;
            slot_bits[sl][i>>6] |= 1ULL << (i & 63);
            int idx = sl + 1;
            count_slot[idx]++;
            if (idx > curMaxSlot) curMaxSlot = idx;
        }
    }
}

static void randomValidAssignment(int order[]) {
    buildUsageFromSolution();
    int limit = bestCost - 1;
    if (limit < 1) limit = MAX_SLOT;
    for (int i = 0; i < N; i++) s[i] = r[i] = 0;
    for (int idx = 0; idx < N; idx++) {
        int cid = order[idx];
        for (int t = 0; t < RANDOM_TRIALS; t++) {
            int sl = rand() % limit;
            if (!canAssign(cid, sl+1)) continue;
            int rm = rand() % M;
            if (c[rm] < d[cid] || room_ts[rm][sl] == epoch) continue;
            applyAssign(cid, rm, sl);
            s[cid] = sl+1;
            r[cid] = rm+1;
            break;
        }
    }
    if (curMaxSlot < bestCost) bestCost = curMaxSlot;
}

static void localSearch() {
    bool improved = true;
    while (improved) {
        improved = false;
        buildUsageFromSolution();
        for (int i = 0; i < N; i++) {
            int oldSl = s[i];
            for (int ns = 1; ns < oldSl; ns++) {
                if (!canAssign(i, ns)) continue;
                int oldRm = r[i] - 1;
                int oldSlot = oldSl - 1;
                for (int nr = 0; nr < M; nr++) {
                    if (c[nr] < d[i] || room_ts[nr][ns-1] == epoch) continue;
                    removeAssign(i, oldRm, oldSlot);
                    applyAssign(i, nr, ns-1);
                    s[i] = ns;
                    r[i] = nr+1;
                    improved = true;
                    goto end_ls;
                }
            }
        }
    end_ls: ;
    }
    if (curMaxSlot < bestCost) bestCost = curMaxSlot;
}

static void perturb() {
    for (int k = 0; k < PERTURB_K; k++) {
        int cid = rand() % N;
        int oldSl = s[cid] - 1;
        int oldRm = r[cid] - 1;
        if (oldSl >= 0 && oldRm >= 0) removeAssign(cid, oldRm, oldSl);
        for (int t = 0; t < RANDOM_TRIALS; t++) {
            int sl = rand() % MAX_SLOT;
            if (!canAssign(cid, sl+1)) continue;
            int rm = rand() % M;
            if (c[rm] < d[cid] || room_ts[rm][sl] == epoch) continue;
            applyAssign(cid, rm, sl);
            s[cid] = sl+1;
            r[cid] = rm+1;
            break;
        }
    }
}

static inline void randomShuffle(int *arr, int n) {
    for (int i = n-1; i > 0; i--) {
        int j = rand() % (i+1);
        int t = arr[i]; arr[i] = arr[j]; arr[j] = t;
    }
}

int main() {
    srand((unsigned)time(NULL));
    scanf("%d %d", &N, &M);
    for (int i = 0; i < N; i++) scanf("%d", &d[i]);
    for (int j = 0; j < M; j++) scanf("%d", &c[j]);
    scanf("%d", &K);
    for (int k = 0; k < K; k++) {
        int u,v; scanf("%d %d", &u,&v); u--; v--;
        conflict_bits[u][v>>6] |= 1ULL << (v & 63);
        conflict_bits[v][u>>6] |= 1ULL << (u & 63);
    }
    int order[MAX_N];
    for (int i = 0; i < N; i++) order[i] = i;

    randomShuffle(order, N);
    randomValidAssignment(order);
    localSearch();
    memcpy(currS, s, sizeof(int)*N);
    memcpy(currR, r, sizeof(int)*N);
    bestCost = curMaxSlot;
    memcpy(bestS, s, sizeof(int)*N);
    memcpy(bestR, r, sizeof(int)*N);

    for (int it = 0; it < ILS_ITERS; it++) {
        memcpy(s, currS, sizeof(int)*N);
        memcpy(r, currR, sizeof(int)*N);
        buildUsageFromSolution();
        perturb();
        localSearch();
        memcpy(currS, s, sizeof(int)*N);
        memcpy(currR, r, sizeof(int)*N);
    }

    for (int i = 0; i < N; i++) printf("%d %d %d\n", i+1, bestS[i], bestR[i]);
    return 0;
}
