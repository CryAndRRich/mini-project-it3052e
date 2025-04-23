"""
Thuật toán tương tự nhưng cải tiến so với mục 3. gồm các phần:
- Giảm không gian tìm kiếm, sử dụng limit động, thay vì luôn duyệt MAX_SLOT = 4*N thì duyệt đến số khung thi hiện có
- Dùng bitset cho xung đột các lớp không thi cùng một khung thời gian và lưu trữ khung thời gian
=> Kiểm tra xung đột giữa lớp và toàn bộ lớp đang ở slot chỉ là một phép AND và cập nhật khi gán/xóa lớp cũng chỉ là thao tác bit OR/AND
=> Thời gian xử lý từ O(N) xuống O(N/64), từ N xấp xỉ 1000 xuống N/64 xấp xỉ 16

Điểm: 99 93 97 100 60
"""
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
#define RESTART_LIMIT 1000
#define W ((MAX_N + 63) / 64)

int N, M, K;
int d[MAX_N];
int c[MAX_M];
uint64_t conflict_bits[MAX_N][W];
int s[MAX_N];
int r[MAX_N];
int room_usage[MAX_M][MAX_SLOT];
uint64_t slot_bits[MAX_SLOT][W];
int bestS[MAX_N], bestR[MAX_N];
int bestCost = INF;

bool canAssign(int class_id, int new_slot) {
    uint64_t *cb = conflict_bits[class_id];
    uint64_t *sb = slot_bits[new_slot-1];
    for (int w = 0; w < W; w++)
        if (cb[w] & sb[w]) return false;
    return true;
}

int computeCost() {
    int cost = 0;
    for (int i = 0; i < N; i++)
        if (s[i] > cost) cost = s[i];
    return cost;
}

void clearRoomUsage() {
    for (int i = 0; i < M; i++)
        for (int j = 0; j < MAX_SLOT; j++)
            room_usage[i][j] = -1;
    for (int k = 0; k < MAX_SLOT; k++)
        for (int w = 0; w < W; w++)
            slot_bits[k][w] = 0;
}

bool greedyAssignment(int order[]) {
    clearRoomUsage();
    for (int i = 0; i < N; i++) {
        s[i] = 0;
        r[i] = 0;
    }
    int slot_limit = bestCost - 1;
    if (slot_limit > MAX_SLOT) slot_limit = MAX_SLOT;
    for (int idx = 0; idx < N; idx++) {
        int cid = order[idx];
        bool assigned = false;
        for (int room_id = 0; room_id < M && !assigned; room_id++) {
            if (c[room_id] < d[cid]) continue;
            for (int slot = 0; slot < slot_limit; slot++) {
                if (room_usage[room_id][slot] != -1) continue;
                if (!canAssign(cid, slot + 1)) continue;
                s[cid] = slot + 1;
                r[cid] = room_id + 1;
                room_usage[room_id][slot] = cid;
                slot_bits[slot][cid>>6] |= 1ULL << (cid & 63);
                assigned = true;
                break;
            }
        }
        if (!assigned) {
            return false;
        }
    }
    return true;
}

void localSearch() {
    bool improvement = true;
    int iter = 0;
    while (improvement && iter < 10000) {
        improvement = false;
        iter++;
        for (int i = 0; i < N; i++) {
            int cur = s[i];
            bool moved = false;
            for (int new_slot = 1; new_slot < cur && !moved; new_slot++) {
                for (int room_id = 0; room_id < M; room_id++) {
                    if (c[room_id] < d[i]) continue;
                    if (room_usage[room_id][new_slot-1] != -1) continue;
                    if (!canAssign(i, new_slot)) continue;
                    int old_room = r[i] - 1;
                    int old_slot = s[i] - 1;
                    room_usage[old_room][old_slot] = -1;
                    slot_bits[old_slot][i>>6] &= ~(1ULL << (i & 63));
                    s[i] = new_slot;
                    r[i] = room_id + 1;
                    room_usage[room_id][new_slot-1] = i;
                    slot_bits[new_slot-1][i>>6] |= 1ULL << (i & 63);
                    improvement = true;
                    moved = true;
                    break;
                }
            }
            if (moved) continue;
        }
    }
}

void randomShuffle(int *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int main() {
    srand(time(NULL));
    scanf("%d %d", &N, &M);
    for (int i = 0; i < N; i++) scanf("%d", &d[i]);
    for (int i = 0; i < M; i++) scanf("%d", &c[i]);
    scanf("%d", &K);
    for (int i = 0; i < K; i++) {
        int u, v;
        scanf("%d %d", &u, &v);
        u--; v--;
        conflict_bits[u][v>>6] |= 1ULL << (v & 63);
        conflict_bits[v][u>>6] |= 1ULL << (u & 63);
    }
    int order[MAX_N];
    for (int i = 0; i < N; i++) order[i] = i;
    for (int restart = 0; restart < RESTART_LIMIT; restart++) {
        randomShuffle(order, N);
        if (!greedyAssignment(order)) continue;
        localSearch();
        int cost = computeCost();
        if (cost < bestCost) {
            bestCost = cost;
            for (int i = 0; i < N; i++) {
                bestS[i] = s[i];
                bestR[i] = r[i];
            }
        }
    }
    for (int i = 0; i < N; i++)
        printf("%d %d %d\n", i+1, bestS[i], bestR[i]);
    return 0;
}
