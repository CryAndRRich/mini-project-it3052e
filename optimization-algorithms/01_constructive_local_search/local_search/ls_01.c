/*
Khởi tạo 1000 hoán vị khác nhau của N lớp một cách ngẫu nhiên, với mỗi cách hoán vị:
- Sử dụng thuật toán Tham Lam để tạo ra 1 cách sắp xếp phòng thi chấp nhận được
- Sử dụng thuật toán Tìm Kiếm Cục Bộ di chuyển lớp về khung thời gian thi nhỏ hơn đến khi không thể nữa

Điểm: 100 93 100 100 62
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAX_N 1000
#define MAX_M 1000
#define MAX_K 10000
#define MAX_SLOT (MAX_N * 4)
#define INF (MAX_SLOT + 1)
#define RESTART_LIMIT 1000

int N, M, K;
int d[MAX_N];
int c[MAX_M];
bool conflict[MAX_N][MAX_N];

int s[MAX_N];
int r[MAX_N];
int room_usage[MAX_M][MAX_SLOT];

int bestS[MAX_N], bestR[MAX_N];
int bestCost = INF;

bool canAssign(int class_id, int new_slot) {
    for (int j = 0; j < N; j++) {
        if (conflict[class_id][j] && s[j] == new_slot)
            return false;
    }
    return true;
}

int computeCost() {
    int cost = 0;
    for (int i = 0; i < N; i++) {
        if (s[i] > cost)
            cost = s[i];
    }
    return cost;
}

void clearRoomUsage() {
    for (int i = 0; i < M; i++)
        for (int j = 0; j < MAX_SLOT; j++)
            room_usage[i][j] = -1;
}

bool greedyAssignment(int order[]) {
    clearRoomUsage();
    for (int i = 0; i < N; i++) {
        s[i] = 0;
        r[i] = 0;
    }

    for (int idx = 0; idx < N; idx++) {
        int class_id = order[idx];
        int bestSlot = INF;
        int bestRoom = -1;
        for (int room_id = 0; room_id < M; room_id++) {
            if (c[room_id] < d[class_id])
                continue;
            for (int slot = 0; slot < MAX_SLOT; slot++) {
                if (room_usage[room_id][slot] != -1)
                    continue;
                if (!canAssign(class_id, slot + 1))
                    continue;
                if (slot + 1 < bestSlot) {
                    bestSlot = slot + 1;
                    bestRoom = room_id;
                }
                break;
            }
        }
        if (bestRoom == -1) {
            return false;
        }
        s[class_id] = bestSlot;
        r[class_id] = bestRoom + 1;
        room_usage[bestRoom][bestSlot - 1] = class_id;
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
            int current_slot = s[i];
            bool foundImprovementForClass = false;
            for (int new_slot = 1; new_slot < current_slot && !foundImprovementForClass; new_slot++) {
                for (int room_id = 0; room_id < M; room_id++) {
                    if (c[room_id] < d[i])
                        continue;
                    if (room_usage[room_id][new_slot - 1] != -1)
                        continue;
                    if (!canAssign(i, new_slot))
                        continue;
                    int old_room = r[i] - 1;
                    int old_slot = s[i] - 1;
                    room_usage[old_room][old_slot] = -1;
                    s[i] = new_slot;
                    r[i] = room_id + 1;
                    room_usage[room_id][new_slot - 1] = i;
                    improvement = true;
                    foundImprovementForClass = true;
                    break;
                }
            }
        }
    }
}

void randomShuffle(int arr[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

int main() {
    srand(time(NULL));

    scanf("%d %d", &N, &M);
    for (int i = 0; i < N; i++) {
        scanf("%d", &d[i]);
    }
    for (int i = 0; i < M; i++) {
        scanf("%d", &c[i]);
    }
    scanf("%d", &K);
    for (int i = 0; i < K; i++) {
        int u, v;
        scanf("%d %d", &u, &v);
        conflict[u - 1][v - 1] = true;
        conflict[v - 1][u - 1] = true;
    }

    int order[MAX_N];
    for (int i = 0; i < N; i++) {
        order[i] = i;
    }

    int tempS[MAX_N], tempR[MAX_N];

    for (int restart = 0; restart < RESTART_LIMIT; restart++) {
        randomShuffle(order, N);
        bool feasible = greedyAssignment(order);
        if (!feasible)
            continue;
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

    for (int i = 0; i < N; i++) {
        printf("%d %d %d\n", i + 1, bestS[i], bestR[i]);
    }

    return 0;
}
