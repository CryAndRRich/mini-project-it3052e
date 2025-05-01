//C 
//C 
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>

#define Max_N 1000
#define Max_M 1000
#define Max_K 100
#define Max_Slot (4 * Max_N)
#define Max_ants 10
#define Max_loops 100

int N, M, K;
int d[Max_N];
int c[Max_M];


#define BIT_WORDS ((Max_N + 63) / 64)
static uint64_t conflictBits[Max_N][BIT_WORDS];


double alpha = 1.0;
double beta  = 2.0;
double rho   = 0.1;
double Q     = 500.0;

double pheromone[Max_N][Max_Slot];
int solSlot[Max_ants][Max_N];
int solRoom[Max_ants][Max_N];
int costAnt[Max_ants];

int bestSlot[Max_N];
int bestRoom[Max_N];
int bestCost;

static inline bool isConflict(int i, int j) {
    return (conflictBits[i][j >> 6] >> (j & 63)) & 1;
}

static inline void setConflict(int i, int j) {
    conflictBits[i][j >> 6] |= 1ULL << (j & 63);
}

static inline int CostCompute(int slotAssign[]) {
    int cost = 0;
    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            if (slotAssign[i] == slotAssign[j] && isConflict(i, j)) cost++;
        }
    }
    return cost;
}

static inline void pheromone_init(double first) {
    for (int i = 0; i < N; i++) {
        for (int s = 0; s < N; s++) {
            pheromone[i][s] = first;
        }
    }
}

void Solution(int k) {
    static bool UsedRoom[Max_M][Max_N];
    for (int r = 0; r < M; r++) {
        for (int s = 0; s < N; s++) UsedRoom[r][s] = false;
    }
    for (int i = 0; i < N; i++) {
        int chosenSlot = 0;
        double bestValue = -1.0;
        for (int s = 0; s < N; s++) {
            bool hasRoom = false;
            for (int r = 0; r < M; r++) {
                if (!UsedRoom[r][s] && c[r] >= d[i]) { hasRoom = true; break; }
            }
            if (!hasRoom) continue;
            int confCount = 0;
            for (int j = 0; j < i; j++) {
                if (solSlot[k][j] == s + 1 && isConflict(i, j)) confCount++;
            }
            double eta = 1.0 / (1 + confCount);
            double value = pow(pheromone[i][s], alpha) * pow(eta, beta);
            if (value > bestValue) { bestValue = value; chosenSlot = s; }
        }
        solSlot[k][i] = chosenSlot + 1;
        for (int r = 0; r < M; r++) {
            if (!UsedRoom[r][chosenSlot] && c[r] >= d[i]) {
                UsedRoom[r][chosenSlot] = true;
                solRoom[k][i] = r + 1;
                break;
            }
        }
    }
    costAnt[k] = CostCompute(solSlot[k]);
}

void update() {
    for (int i = 0; i < N; i++) {
        for (int s = 0; s < N; s++) pheromone[i][s] *= (1 - rho);
    }
    int bestK = 0;
    for (int k = 1; k < Max_ants; k++) if (costAnt[k] < costAnt[bestK]) bestK = k;
    for (int i = 0; i < N; i++) {
        int s = solSlot[bestK][i] - 1;
        pheromone[i][s] += Q / (1 + costAnt[bestK]);
    }
    if (costAnt[bestK] < bestCost) {
        bestCost = costAnt[bestK];
        for (int i = 0; i < N; i++) {
            bestSlot[i] = solSlot[bestK][i];
            bestRoom[i] = solRoom[bestK][i];
        }
    }
}

void ACO() {
    bestCost = INT_MAX;
    pheromone_init(1.0);
    for (int loop = 0; loop < Max_loops; loop++) {
        for (int k = 0; k < Max_ants; k++) Solution(k);
        update();
    }
    for (int i = 0; i < N; i++) printf("%d %d %d\n", i + 1, bestSlot[i], bestRoom[i]);
}

int main() {
    scanf("%d %d", &N, &M);
    for (int i = 0; i < N; i++) scanf("%d", &d[i]);
    for (int j = 0; j < M; j++) scanf("%d", &c[j]);
    scanf("%d", &K);
    for (int k = 0; k < K; k++) {
        int u, v;
        scanf("%d %d", &u, &v);
        setConflict(u-1, v-1);
        setConflict(v-1, u-1);
    }
    ACO();
    return 0;
}
