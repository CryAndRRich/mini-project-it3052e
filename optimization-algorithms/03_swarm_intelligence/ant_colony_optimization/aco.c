/*
Thuật toán kiến

Điểm: TLE 93 TLE 100 60
*/
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#define Max_N 1000
#define Max_M 1000
#define Max_K 100
#define Max_Slot (4 * Max_N)
#define Max_ants 20
#define Max_loops 100

int N, M, K;
int d[Max_N];
int c[Max_M];
bool conflict[Max_N][Max_N];

double alpha = 1.0;
double beta = 2.0;
double rho = 0.1;
double Q = 500.0;

double pheromone[Max_N][Max_Slot];
int solSlot[Max_ants][Max_N];
int solRoom[Max_ants][Max_N];
int costAnt[Max_ants];

double bestPheromone;
int bestSlot[Max_N];
int bestRoom[Max_N];
int bestCost;


bool UsedRoom[Max_M][Max_Slot];

int CostCompute(int slotAssign[]) {
    int cost = 0;
    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            if (slotAssign[i] == slotAssign[j] && conflict[i][j]) cost++;
        }
    }
    return cost;
}

void pheromone_init(double first) {
    for (int i = 0; i < N; i++) {
        for (int s = 0; s < N; s++) pheromone[i][s] = first;
    }
}

void Solution(int k) {
    for (int r = 0; r < M; r++) {
        for (int s = 0; s < N; s++) {
            UsedRoom[r][s] = false;
        }
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
                if (solSlot[k][j] == s + 1 && conflict[i][j]) confCount++;
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
        conflict[u-1][v-1] = conflict[v-1][u-1] = true;
    }
    ACO();
    return 0;
}
