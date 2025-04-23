"""
Thay vì xét thứ tự từ 1 đến N thì sắp xếp trước các lớp theo sĩ số lớp và số lớp mà lớp đó không thể thi chung một khung thời gian (số lớp mâu thuẫn)
Ý tưởng chính là vì các lớp sĩ số lớn và số lớp mâu thuẫn lớn sẽ khó xếp hơn nên sẽ ưu tiên xếp trước

Điểm: 100 90 100 66 60
"""
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_N 1000
#define MAX_M 1000
#define MAX_K 10000
#define MAX_SLOT (MAX_N * 4)
#define INF (MAX_SLOT + 1)

int N, M, K;
int d[MAX_N];
int c[MAX_M];
bool conflict[MAX_N][MAX_N];

int s[MAX_N];
int r[MAX_N];

int room_usage[MAX_M][MAX_SLOT];

int conflictCount[MAX_N];
int order[MAX_N];

int compare(const void *a, const void *b) {
    int i = *(int *)a;
    int j = *(int *)b;
    if (conflictCount[j] != conflictCount[i])
        return conflictCount[j] - conflictCount[i];
    return d[j] - d[i];
}

int main() {
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

    for (int i = 0; i < M; i++)
        for (int j = 0; j < MAX_SLOT; j++)
            room_usage[i][j] = -1;

    for (int i = 0; i < N; i++) {
        conflictCount[i] = 0;
        for (int j = 0; j < N; j++) {
            if (conflict[i][j])
                conflictCount[i]++;
        }
        order[i] = i;
    }

    qsort(order, N, sizeof(int), compare);

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
                bool conflict_found = false;
                for (int other = 0; other < N; other++) {
                    if (conflict[class_id][other] && s[other] == slot + 1) {
                        conflict_found = true;
                        break;
                    }
                }
                if (!conflict_found) {
                    if (slot + 1 < bestSlot) {
                        bestSlot = slot + 1;
                        bestRoom = room_id;
                    }
                    break;
                }
            }
        }
        s[class_id] = bestSlot;
        r[class_id] = bestRoom + 1;
        room_usage[bestRoom][bestSlot - 1] = class_id;
    }

    for (int i = 0; i < N; i++) {
        printf("%d %d %d\n", i + 1, s[i], r[i]);
    }

    return 0;
}