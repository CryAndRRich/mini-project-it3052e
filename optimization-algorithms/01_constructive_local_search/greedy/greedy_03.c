/*
Thuật toán Tham Lam + Khởi đầu thứ tự ngẫu nhiên

Điểm: 100 93 100 100 71
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAX_N 1000
#define MAX_M 1000
#define MAX_K 10000
#define MAX_SLOT (MAX_N * 4)
#define RANDOM_ITERS 1000
#define INF (MAX_SLOT + 1)

int N, M, K;
int d[MAX_N];
int c[MAX_M];
bool conflict[MAX_N][MAX_N];

int s[MAX_N], r[MAX_N];
int bestS[MAX_N], bestR[MAX_N];   
int bestCost = INF;

int room_usage[MAX_M][MAX_SLOT];
int order[MAX_N];                 

static void greedyAssign() {
    for (int i = 0; i < N; i++) {
        s[i] = 0;
        r[i] = 0;
    }
    for (int rm = 0; rm < M; rm++)
        for (int sl = 0; sl < MAX_SLOT; sl++)
            room_usage[rm][sl] = -1;

    for (int idx = 0; idx < N; idx++) {
        int cid = order[idx];
        int bestSlot = INF;
        int bestRoom = -1;
        for (int rm = 0; rm < M; rm++) {
            if (c[rm] < d[cid]) continue;
            for (int sl = 0; sl < MAX_SLOT; sl++) {
                if (room_usage[rm][sl] != -1) continue;
                bool ok = true;
                for (int other = 0; other < N; other++) {
                    if (conflict[cid][other] && s[other] == sl+1) {
                        ok = false;
                        break;
                    }
                }
                if (!ok) continue;
                if (sl+1 < bestSlot) {
                    bestSlot = sl+1;
                    bestRoom = rm;
                }
                break; 
            }
        }
        if (bestRoom >= 0) {
            s[cid] = bestSlot;
            r[cid] = bestRoom + 1;
            room_usage[bestRoom][bestSlot - 1] = cid;
        } else {
            s[cid] = INF;
            r[cid] = 1;
        }
    }
}

static int computeCost() {
    int cost = 0;
    for (int i = 0; i < N; i++) {
        if (s[i] > cost) cost = s[i];
    }
    return cost;
}

static void randomShuffle() {
    for (int i = N-1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
    }
}

int main() {
    srand((unsigned)time(NULL));
    scanf("%d %d", &N, &M);
    for (int i = 0; i < N; i++) scanf("%d", &d[i]);
    for (int j = 0; j < M; j++) scanf("%d", &c[j]);
    scanf("%d", &K);
    for (int k = 0; k < K; k++) {
        int u,v; scanf("%d %d", &u, &v);
        conflict[u-1][v-1] = conflict[v-1][u-1] = true;
    }
    
    for (int i = 0; i < N; i++) order[i] = i;

    for (int it = 0; it < RANDOM_ITERS; it++) {
        randomShuffle();
        greedyAssign();
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
        printf("%d %d %d\n", i+1, bestS[i], bestR[i]);
    }
    return 0;
}
