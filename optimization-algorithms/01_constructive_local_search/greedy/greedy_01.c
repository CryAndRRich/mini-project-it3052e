"""
Xét theo thứ tự các lớp từ 1 đến N, mỗi lần xét gán cho lớp i phòng thi nhỏ nhất có sức chứa lớn hơn sĩ số lớp và slot nhỏ nhất đáp ứng các điều kiện

Điểm: 100 93 96 100 60
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

    for (int class_id = 0; class_id < N; class_id++) {
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