"""
Một vài cải tiến so với mục 7. là:
- Sau khi chạy Luyện Kim Mô Phỏng cho ra một kết quả tốt, chạy thêm một lần Tìm Kiếm Cục Bộ để đảm bảo có được kết quả ổn nhất
- Dùng timestamp-based reset để tiết kiệm memset
- Gắn static inline cho các hàm quan trọng để giảm overhead
- Thay vì mỗi lần gọi hàm computeCost để tính thì track cost incremental
=> Tiếp tục giảm phần lớn thời gian chạy của chương trình
=> Tạo điều kiện để nới rộng khoảng của Luyện Kim Mô Phỏng (Tmin 0.01 -> 0.000001 và alpha 0.999 -> 0.999999) mà không bị giới hạn thời gian

Điểm: 91 94 90 100 100
"""
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>

#define MAX_N 1000
#define MAX_M 1000
#define MAX_SLOT (MAX_N * 4)
#define INF (MAX_SLOT + 1)
#define RESTART_LIMIT 1000
#define RANDOM_TRIALS 100
#define W ((MAX_N + 63) / 64)

int N, M, K;
int d[MAX_N];
int c[MAX_M];
uint64_t conflict_bits[MAX_N][W];

int room_ts[MAX_M][MAX_SLOT];
int room_usage[MAX_M][MAX_SLOT];

int slot_ts[MAX_SLOT][W];
uint64_t slot_bits[MAX_SLOT][W];

int bestS[MAX_N], bestR[MAX_N];
int bestCost = INF;
int SLOT_LIMIT;
int epoch = 1;

int count_slot[MAX_SLOT + 1];
int curMaxSlot;

static inline bool canAssign(int cid, int slot) {
    uint64_t *cb = conflict_bits[cid];
    int idx = slot - 1;
    for (int w = 0; w < W; w++) {
        uint64_t sb = (slot_ts[idx][w] == epoch ? slot_bits[idx][w] : 0ULL);
        if (cb[w] & sb) return false;
    }
    return true;
}

static inline void applyAssign(int cid, int room, int slot) {
    room_ts[room][slot] = epoch;
    room_usage[room][slot] = cid;
    int w = cid >> 6, b = cid & 63;
    slot_ts[slot][w] = epoch;
    slot_bits[slot][w] |= (1ULL << b);
    int cnt_idx = slot + 1;
    count_slot[cnt_idx]++;
    if (cnt_idx > curMaxSlot) curMaxSlot = cnt_idx;
}

static inline void removeAssign(int cid, int room, int slot) {
    room_ts[room][slot] = -epoch;
    int w = cid >> 6, b = cid & 63;
    if (slot_ts[slot][w] == epoch) slot_bits[slot][w] &= ~(1ULL << b);
    int cnt_idx = slot + 1;
    if (--count_slot[cnt_idx] == 0 && cnt_idx == curMaxSlot) {
        while (curMaxSlot > 0 && count_slot[curMaxSlot] == 0) curMaxSlot--;
    }
}

static inline void clearUsage() {
    epoch++;
    for (int i = 1; i <= SLOT_LIMIT; i++) count_slot[i] = 0;
    curMaxSlot = 0;
}

bool greedyAssignment(int order[]) {
    clearUsage();
    int tempS[MAX_N], tempR[MAX_N];
    for (int i = 0; i < N; i++) tempS[i] = tempR[i] = 0;
    int cap = bestCost - 1;
    if (cap > SLOT_LIMIT) cap = SLOT_LIMIT;

    for (int idx = 0; idx < N; idx++) {
        int cid = order[idx];
        bool done = false;
        for (int rm = 0; rm < M && !done; rm++) {
            if (c[rm] < d[cid]) continue;
            for (int sl = 0; sl < cap; sl++) {
                if (room_ts[rm][sl] == epoch) continue;
                if (!canAssign(cid, sl+1)) continue;
                tempS[cid] = sl+1;
                tempR[cid] = rm+1;
                applyAssign(cid, rm, sl);
                done = true;
                break;
            }
        }
        if (!done) return false;
    }
    for (int i = 0; i < N; i++) {
        bestS[i] = tempS[i];
        bestR[i] = tempR[i];
    }
    bestCost = curMaxSlot;
    return true;
}

void simulatedAnnealing() {
    int curS[MAX_N], curR[MAX_N];
    for (int i = 0; i < N; i++) {
        curS[i] = bestS[i];
        curR[i] = bestR[i];
    }
    clearUsage();
    for (int i = 0; i < N; i++) {
        if (curS[i] && curR[i]) applyAssign(i, curR[i]-1, curS[i]-1);
    }
    int curCost = curMaxSlot;

    double T = 100.0, Tmin = 0.000001, alpha = 0.999999;
    while (T > Tmin) {
        int maxCid = 0;
        for (int i = 1; i < N; i++) if (curS[i] > curS[maxCid]) maxCid = i;
        int cid = (rand() & 1 ? maxCid : rand() % N);
        int oldS = curS[cid]-1, oldR = curR[cid]-1;
        removeAssign(cid, oldR, oldS);

        bool placed = false;
        int newS, newR;
        for (int t = 0; t < RANDOM_TRIALS; t++) {
            newS = rand() % SLOT_LIMIT;
            newR = rand() % M;
            if (c[newR] < d[cid] || room_ts[newR][newS] == epoch) continue;
            if (!canAssign(cid, newS+1)) continue;
            placed = true;
            break;
        }
        if (!placed) {
            applyAssign(cid, oldR, oldS);
            T *= alpha;
            continue;
        }
        curS[cid] = newS+1;
        curR[cid] = newR+1;
        applyAssign(cid, newR, newS);
        int newCost = curMaxSlot;
        int delta = newCost - curCost;
        if (delta < 0 || exp(-delta/T) > ((double)rand()/RAND_MAX)) {
            curCost = newCost;
            if (curCost < bestCost) {
                bestCost = curCost;
                for (int i = 0; i < N; i++) {
                    bestS[i] = curS[i];
                    bestR[i] = curR[i];
                }
            }
        } else {
            removeAssign(cid, newR, newS);
            curS[cid] = oldS+1;
            curR[cid] = oldR+1;
            applyAssign(cid, oldR, oldS);
        }
        T *= alpha;
    }
}

void localSearch() {
    bool improved = true;
    while (improved) {
        improved = false;
        clearUsage();
        for (int i = 0; i < N; i++) {
            if (bestS[i] && bestR[i])
                applyAssign(i, bestR[i]-1, bestS[i]-1);
        }
        for (int cid = 0; cid < N; cid++) {
            int oldSlot = bestS[cid] - 1;
            int oldRoom = bestR[cid] - 1;
            for (int newSlot = 0; newSlot < oldSlot; newSlot++) {
                for (int newRoom = 0; newRoom < M; newRoom++) {
                    if (c[newRoom] < d[cid]) continue;
                    if (room_ts[newRoom][newSlot] == epoch) continue;
                    if (!canAssign(cid, newSlot+1)) continue;
                    removeAssign(cid, oldRoom, oldSlot);
                    applyAssign(cid, newRoom, newSlot);
                    int newCost = curMaxSlot;
                    if (newCost < bestCost) {
                        bestCost = newCost;
                        bestS[cid] = newSlot+1;
                        bestR[cid] = newRoom+1;
                        improved = true;
                        goto next_iteration;
                    } else {
                        removeAssign(cid, newRoom, newSlot);
                        applyAssign(cid, oldRoom, oldSlot);
                    }
                }
            }
        }
    next_iteration: ;
    }
}

void randomShuffle(int *arr, int n) {
    for (int i = n-1; i > 0; i--) {
        int j = rand() % (i+1);
        int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }
}

int main() {
    srand((unsigned)time(NULL));
    scanf("%d %d", &N, &M);
    SLOT_LIMIT = N;
    for (int i = 0; i < N; i++) scanf("%d", &d[i]);
    for (int i = 0; i < M; i++) scanf("%d", &c[i]);
    scanf("%d", &K);
    for (int i = 0; i < K; i++) {
        int u, v; scanf("%d %d", &u, &v);
        u--; v--;
        conflict_bits[u][v>>6] |= 1ULL << (v & 63);
        conflict_bits[v][u>>6] |= 1ULL << (u & 63);
    }
    int order[MAX_N];
    for (int i = 0; i < N; i++) order[i] = i;

    for (int t = 0; t < RESTART_LIMIT; t++) {
        randomShuffle(order, N);
        if (!greedyAssignment(order)) continue;
        simulatedAnnealing();
        localSearch();
    }

    for (int i = 0; i < N; i++) printf("%d %d %d\n", i+1, bestS[i], bestR[i]);
    return 0;
}
