"""
Khác với Tìm Kiếm Cục Bộ, cơ chế “chấp nhận cả bước xấu” với xác suất giảm theo nhiệt độ,
Luyện Kim Mô Phỏng giúp thoát khỏi cực tiểu cục bộ và tìm được lời giải tốt hơn

Tuy nhiên để có được kết quả tốt, Luyện Kim Mô Phỏng cần có một khoảng tìm kiếm đủ lớn (Tmin gần 0 và alpha gần 1) và điều này dẫn đến thời gian xử lý
lâu kể cả khi đã dùng kết hợp với bitset

Điểm: 60 69 65 100 71
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
#define RANDOM_TRIALS 1000
#define W ((MAX_N + 63) / 64)

int N, M, K;
int d[MAX_N];
int c[MAX_M];
uint64_t conflict_bits[MAX_N][W];
int s[MAX_N], r[MAX_N];
int room_usage[MAX_M][MAX_SLOT];
uint64_t slot_bits[MAX_SLOT][W];
int bestS[MAX_N], bestR[MAX_N];
int bestCost = INF;
int SLOT_LIMIT;

bool canAssign(int class_id, int new_slot) {
    uint64_t *cb = conflict_bits[class_id];
    uint64_t *sb = slot_bits[new_slot - 1];
    for (int w = 0; w < W; w++) {
        if (cb[w] & sb[w]) return false;
    }
    return true;
}

int computeCost(int arr[]) {
    int cost = 0;
        for (int i = 0; i < N; i++)
            if (arr[i] > cost) cost = arr[i];
        return cost;
}

void clearRoomUsage() {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < SLOT_LIMIT; j++) {
            room_usage[i][j] = -1;
        }
    }
    for (int k = 0; k < SLOT_LIMIT; k++) {
        for (int w = 0; w < W; w++) {
            slot_bits[k][w] = 0;
        }
    }
}

bool greedyAssignment(int order[]) {
    clearRoomUsage();
    for (int i = 0; i < N; i++) { s[i] = r[i] = 0; }
    int slot_cap = bestCost - 1;
    if (slot_cap > SLOT_LIMIT) slot_cap = SLOT_LIMIT;

    for (int idx = 0; idx < N; idx++) {
        int cid = order[idx];
        bool assigned = false;
        for (int room_id = 0; room_id < M && !assigned; room_id++) {
            if (c[room_id] < d[cid]) continue;
            for (int slot = 0; slot < slot_cap; slot++) {
                if (room_usage[room_id][slot] != -1) continue;
                if (!canAssign(cid, slot + 1)) continue;
                s[cid] = slot + 1;
                r[cid] = room_id + 1;
                room_usage[room_id][slot] = cid;
                slot_bits[slot][cid >> 6] |= 1ULL << (cid & 63);
                assigned = true;
                break;
            }
        }
        if (!assigned) return false;
    }
    return true;
}

bool randomValidAssignment(int order[]) {
    clearRoomUsage();
    for (int i = 0; i < N; i++) { s[i] = r[i] = 0; }

    for (int idx = 0; idx < N; idx++) {
        int cid = order[idx];
        bool assigned = false;
        for (int trial = 0; trial < RANDOM_TRIALS && !assigned; trial++) {
            int slot = rand() % SLOT_LIMIT;
            for (int room_id = 0; room_id < M; room_id++) {
                if (c[room_id] < d[cid]) continue;
                if (room_usage[room_id][slot] != -1) continue;
                if (!canAssign(cid, slot + 1)) continue;
                s[cid] = slot + 1;
                r[cid] = room_id + 1;
                room_usage[room_id][slot] = cid;
                slot_bits[slot][cid >> 6] |= 1ULL << (cid & 63);
                assigned = true;
                break;
            }
        }
        if (!assigned) return false;
    }
    return true;
}

void simulatedAnnealing() {
    int curS[MAX_N], curR[MAX_N];

    for (int i = 0; i < N; i++) {
        curS[i] = s[i];
        curR[i] = r[i];
    }
    int curCost = computeCost(curS);

    clearRoomUsage();
    for (int i = 0; i < N; i++) {
        if (curS[i] && curR[i]) {
            int slot = curS[i] - 1;
            int room = curR[i] - 1;
            room_usage[room][slot] = i;
            slot_bits[slot][i >> 6] |= 1ULL << (i & 63);
        }
    }

    double T = 100.0;
    double Tmin = 0.01;
    double alpha = 0.99980108;

    while (T > Tmin) {
        int max_cid = 0;
        for (int i = 1; i < N; i++) {
            if (curS[i] > curS[max_cid])
                max_cid = i;
        }
        int cid = (rand() % 2 == 0) ? max_cid : rand() % N;

        int oldSlot = curS[cid] - 1;
        int oldRoom = curR[cid] - 1;

        room_usage[oldRoom][oldSlot] = -1;
        slot_bits[oldSlot][cid >> 6] &= ~(1ULL << (cid & 63));

        bool placed = false;
        int newSlot, newRoom;

        for (int tries = 0; tries < M; tries++) {
            newSlot = rand() % SLOT_LIMIT;
            newRoom = rand() % M;
            if (c[newRoom] < d[cid]) continue;
            if (room_usage[newRoom][newSlot] != -1) continue;
            if (!canAssign(cid, newSlot + 1)) continue;

            placed = true;
            break;
        }

        if (placed) {
            curS[cid] = newSlot + 1;
            curR[cid] = newRoom + 1;
            room_usage[newRoom][newSlot] = cid;
            slot_bits[newSlot][cid >> 6] |= 1ULL << (cid & 63);
        } else {
            room_usage[oldRoom][oldSlot] = cid;
            slot_bits[oldSlot][cid >> 6] |= 1ULL << (cid & 63);
            T *= alpha;
            continue;
        }

        int newCost = computeCost(curS);
        int delta = newCost - curCost;

        if (delta < 0 || exp(-delta / T) > ((double)rand() / RAND_MAX)) {
            curCost = newCost;
            if (curCost < bestCost) {
                bestCost = curCost;
                for (int i = 0; i < N; i++) {
                    bestS[i] = curS[i];
                    bestR[i] = curR[i];
                }
            }
        } else {
            room_usage[newRoom][newSlot] = -1;
            slot_bits[newSlot][cid >> 6] &= ~(1ULL << (cid & 63));
            curS[cid] = oldSlot + 1;
            curR[cid] = oldRoom + 1;
            room_usage[oldRoom][oldSlot] = cid;
            slot_bits[oldSlot][cid >> 6] |= 1ULL << (cid & 63);
        }

        T *= alpha;
    }
}


void randomShuffle(int *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
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
        conflict_bits[u][v >> 6] |= 1ULL << (v & 63);
        conflict_bits[v][u >> 6] |= 1ULL << (u & 63);
    }

    int order[MAX_N];
    for (int i = 0; i < N; i++) order[i] = i;

    for (int restart = 0; restart < RESTART_LIMIT; restart++) {
        randomShuffle(order, N);
        bool ok;
        if (rand() % 2 == 0)
            ok = greedyAssignment(order);
        else
            ok = randomValidAssignment(order);

        if (!ok) continue;
        simulatedAnnealing();
    }

    for (int i = 0; i < N; i++)
        printf("%d %d %d\n", i+1, bestS[i], bestR[i]);

    return 0;
}
