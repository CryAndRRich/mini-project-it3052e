/*
Tabu Search:
Một số cải tiến thời gian:
- Giảm số lượng lớp xét nhờ topClasses chỉ xét lớp có slot lớn.
- Hạn chế phòng cần xét với rooms_for_class.
- Dừng sớm nếu không cải thiện để tiết kiệm thời gian

Điểm: 96 88 96 100 60
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
#define RESTART_LIMIT 100
#define TABU_TENURE 20 
#define MAX_ITER 2000   
#define NO_IMPROVE_LIMIT 50
#define W ((MAX_N + 63) / 64)

int N, M, K;
int d[MAX_N];          
int c[MAX_M]; 
uint64_t conflict_bits[MAX_N][W];

int s[MAX_N], r[MAX_N];     
int room_usage[MAX_M][MAX_SLOT];
uint64_t slot_bits[MAX_SLOT][W];
int slot_count[MAX_SLOT+1]; 

int bestS[MAX_N], bestR[MAX_N];
int bestCost = INF;

typedef struct { int cls, slot; } TabuMove;
static TabuMove tabu_list[TABU_TENURE];
static int tabu_head = 0;

static int rooms_for_class[MAX_N][MAX_M];
static int rooms_count[MAX_N];

static inline bool canAssign(int cls_id, int slot) {
    uint64_t *cb = conflict_bits[cls_id];
    uint64_t *sb = slot_bits[slot-1];
    for (int w = 0; w < W; w++) {
        if (cb[w] & sb[w]) return false;
    }
    return true;
}

static inline void addTabu(int cls_id, int slot) {
    tabu_list[tabu_head] = (TabuMove){cls_id, slot};
    tabu_head = (tabu_head + 1) % TABU_TENURE;
}

static inline bool isTabu(int cls_id, int slot) {
    for (int i = 0; i < TABU_TENURE; i++) {
        if (tabu_list[i].cls == cls_id && tabu_list[i].slot == slot)
            return true;
    }
    return false;
}

static inline bool no_class_at_slot(int slot) {
    return slot_count[slot] == 0;
}

static int find_second_max_slot() {
    for (int slot = bestCost - 1; slot >= 1; slot--) {
        if (slot_count[slot] > 0) return slot;
    }
    return 0;
}

void clearUsage() {
    for (int i = 0; i < M; i++)
        for (int j = 0; j < MAX_SLOT; j++)
            room_usage[i][j] = -1;
    for (int k = 0; k < MAX_SLOT; k++) {
        for (int w = 0; w < W; w++) slot_bits[k][w] = 0;
        slot_count[k+1] = 0;
    }
}

bool randomValidAssignment(int order[]) {
    clearUsage();
    for (int i = 0; i < N; i++) {
        s[i] = 0;
        r[i] = 0;
    }

    for (int idx = 0; idx < N; idx++) {
        int cid = order[idx];
        bool assigned = false;

        for (int trial = 0; trial < 100 && !assigned; trial++) {
            int slot = rand() % MAX_SLOT;

            for (int j = 0; j < rooms_count[cid]; j++) {
                int room_id = rooms_for_class[cid][j];
                if (room_usage[room_id][slot] != -1) continue;
                if (!canAssign(cid, slot + 1)) continue;

                s[cid] = slot + 1;
                r[cid] = room_id + 1;
                room_usage[room_id][slot] = cid;
                slot_bits[slot][cid>>6] |= 1ULL << (cid & 63);
                slot_count[slot + 1]++;
                assigned = true;
                break;
            }
        }

        if (!assigned) return false;
    }

    return true;
}

bool greedyAssignment(int order[]) {
    clearUsage();
    for (int i = 0; i < N; i++) s[i] = r[i] = 0;
    int limit = bestCost < MAX_SLOT ? bestCost-1 : MAX_SLOT;
    for (int idx = 0; idx < N; idx++) {
        int cls = order[idx]; bool done = false;
        for (int j = 0; j < rooms_count[cls] && !done; j++) {
            int rm = rooms_for_class[cls][j];
            for (int slot = 1; slot <= limit; slot++) {
                if (room_usage[rm][slot-1] != -1) continue;
                if (!canAssign(cls, slot)) continue;
                s[cls] = slot; r[cls] = rm+1;
                room_usage[rm][slot-1] = cls;
                slot_bits[slot-1][cls>>6] |= 1ULL << (cls & 63);
                slot_count[slot]++;
                done = true;
                break;
            }
        }
        if (!done) return false;
    }
    return true;
}

void tabuSearch(int order[]) {
    clearUsage();
    bool ok = (rand() & 1) ? greedyAssignment(order) : randomValidAssignment(order);
    if (!ok) return;

    int currentCost = 0;
    for (int i = 0; i < N; i++) if (s[i] > currentCost) currentCost = s[i];
    bestCost = currentCost;
    for (int i = 0; i < N; i++) bestS[i] = s[i], bestR[i] = r[i];

    int iter = 0, noImprove = 0;

    while (iter++ < MAX_ITER && noImprove < NO_IMPROVE_LIMIT) {
        int topClasses[MAX_N];
        int topCount = 0;
        for (int i = 0; i < N; i++) {
            if (s[i] >= currentCost - 1) topClasses[topCount++] = i;
        }

        int bestNbCost = INF, bi=-1, bslot=-1, broom=-1;

        for (int t = 0; t < topCount; t++) {
            int cls = topClasses[t];
            int old_slot = s[cls];

            for (int slot = 1; slot < bestCost; slot++) {
                if (slot == old_slot || !canAssign(cls, slot)) continue;

                for (int j = 0; j < rooms_count[cls]; j++) {
                    int rm = rooms_for_class[cls][j];
                    if (room_usage[rm][slot-1] != -1) continue;
                    if (isTabu(cls, slot) && currentCost >= bestCost) continue;

                    int newCost = slot > currentCost ? slot :
                        (old_slot == currentCost && slot_count[currentCost] == 1) ? find_second_max_slot() : currentCost;

                    if (newCost < bestNbCost) {
                        bestNbCost = newCost;
                        bi = cls; bslot = slot; broom = rm;
                    }
                }
            }
        }

        if (bi < 0) break;

        int old_slot = s[bi], old_room = r[bi]-1;
        room_usage[old_room][old_slot-1] = -1;
        slot_bits[old_slot-1][bi>>6] &= ~(1ULL << (bi & 63));
        slot_count[old_slot]--;

        s[bi] = bslot; r[bi] = broom+1;
        room_usage[broom][bslot-1] = bi;
        slot_bits[bslot-1][bi>>6] |= 1ULL << (bi & 63);
        slot_count[bslot]++;

        addTabu(bi, old_slot);
        currentCost = bestNbCost;

        if (currentCost < bestCost) {
            bestCost = currentCost;
            for (int i = 0; i < N; i++) bestS[i] = s[i], bestR[i] = r[i];
            noImprove = 0;
        } else {
            noImprove++;
        }
    }
}

void randomShuffle(int *a, int n) {
    for (int i = n-1; i > 0; i--) {
        int j = rand() % (i+1);
        int t = a[i]; a[i] = a[j]; a[j] = t;
    }
}

int main() {
    srand((unsigned)time(NULL));
    scanf("%d %d", &N, &M);
    for (int i = 0; i < N; i++) scanf("%d", &d[i]);
    for (int i = 0; i < M; i++) scanf("%d", &c[i]);
    scanf("%d", &K);
    for (int i = 0; i < K; i++) {
        int u,v; scanf("%d %d", &u,&v); u--; v--;
        conflict_bits[u][v>>6] |= 1ULL << (v & 63);
        conflict_bits[v][u>>6] |= 1ULL << (u & 63);
    }

    for (int i = 0; i < N; i++) {
        rooms_count[i] = 0;
        for (int j = 0; j < M; j++) {
            if (c[j] >= d[i]) {
                rooms_for_class[i][rooms_count[i]++] = j;
            }
        }
    }

    int order[MAX_N];
    for (int i = 0; i < N; i++) order[i] = i;
    for (int t = 0; t < RESTART_LIMIT; t++) {
        randomShuffle(order, N);
        tabuSearch(order);
    }

    for (int i = 0; i < N; i++) printf("%d %d %d\n", i+1, bestS[i], bestR[i]);
    return 0;
}
