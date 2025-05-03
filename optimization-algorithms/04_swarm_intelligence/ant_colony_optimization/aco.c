#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#define max_n 3000
#define max_m 20
#define max_slot (max_n * 4)
#define inf (max_slot + 1)
#define w_words ((max_n + 63) / 64)

#define max_ants 10
#define max_loops 100

double alpha = 1.0;
double beta = 2.0;
double rho = 0.01;
double Q = 100.0;

static uint32_t rng_state;

static inline uint32_t fast_rand(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

int n, m, k;
int course_duration[max_n];
int room_capacity[max_m];

uint64_t conflict_bits[max_n][w_words];

double pheromone[max_n][max_slot];
int sol_slot[max_ants][max_n];
int sol_room[max_ants][max_n];
int cost_ant[max_ants];

int best_slot_assignment[max_n];
int best_room_assignment[max_n];
int best_cost;

static inline bool is_conflict(int i, int j) {
    uint64_t bit = (conflict_bits[i][j >> 6] >> (j & 63)) & 1ULL;
    return bit != 0;
}

static inline void set_conflict(int i, int j) {
    conflict_bits[i][j >> 6] |= 1ULL << (j & 63);
}

static inline int compute_cost(const int slot_assign[]) {
    int cost = 0;
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (slot_assign[i] == slot_assign[j] && is_conflict(i, j)) {
                cost++;
            }
        }
    }
    return cost;
}

static inline void init_pheromone(double init_val) {
    for (int i = 0; i < n; i++) {
        for (int s = 0; s < n; s++) {
            pheromone[i][s] = init_val;
        }
    }
    best_cost = inf;
}

static inline void construct_solution(int a) {
    static bool used_room[max_m][max_slot];
    memset(used_room, 0, sizeof(used_room));
    for (int i = 0; i < n; i++) {
        int chosen_slot = 0;
        double best_value = -1.0;
        for (int s = 0; s < n; s++) {
            bool has_room = false;
            for (int r = 0; r < m; r++) {
                if (!used_room[r][s] && room_capacity[r] >= course_duration[i]) {
                    has_room = true;
                    break;
                }
            }
            if (!has_room) continue;
            int conf_count = 0;
            for (int j = 0; j < i; j++) {
                if (sol_slot[a][j] == s+1 && is_conflict(i, j)) {
                    conf_count++;
                }
            }
            double eta = 1.0 / (1 + conf_count);
            double val = pow(pheromone[i][s], alpha) * pow(eta, beta);
            if (val > best_value) {
                best_value = val;
                chosen_slot = s;
            }
        }
        sol_slot[a][i] = chosen_slot + 1;
        for (int r = 0; r < m; r++) {
            if (!used_room[r][chosen_slot] && room_capacity[r] >= course_duration[i]) {
                used_room[r][chosen_slot] = true;
                sol_room[a][i] = r + 1;
                break;
            }
        }
    }
    cost_ant[a] = compute_cost(sol_slot[a]);
}

static inline void update_pheromone(void) {
    for (int i = 0; i < n; i++) {
        for (int s = 0; s < n; s++) {
            pheromone[i][s] *= (1 - rho);
        }
    }
    int best_a = 0;
    for (int a = 1; a < max_ants; a++) {
        if (cost_ant[a] < cost_ant[best_a]) best_a = a;
    }
    for (int i = 0; i < n; i++) {
        int s = sol_slot[best_a][i] - 1;
        pheromone[i][s] += Q / (1 + cost_ant[best_a]);
    }
    if (cost_ant[best_a] < best_cost) {
        best_cost = cost_ant[best_a];
        for (int i = 0; i < n; i++) {
            best_slot_assignment[i] = sol_slot[best_a][i];
            best_room_assignment[i] = sol_room[best_a][i];
        }
    }
}

static inline void aco(void) {
    init_pheromone(1.0);
    for (int loop = 0; loop < max_loops; loop++) {
        for (int a = 0; a < max_ants; a++) {
            construct_solution(a);
        }
        update_pheromone();
    }
}

void input_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen"); exit(EXIT_FAILURE); }
    fscanf(f, "%d %d", &n, &m);
    for (int i = 0; i < n; i++) fscanf(f, "%d", &course_duration[i]);
    for (int j = 0; j < m; j++) fscanf(f, "%d", &room_capacity[j]);
    fscanf(f, "%d", &k);
    for (int e = 0; e < k; e++) {
        int u, v; fscanf(f, "%d %d", &u, &v);
        u--; v--;
        set_conflict(u, v);
        set_conflict(v, u);
    }
    fclose(f);
}

void input_manual(void) {
    scanf("%d %d", &n, &m);
    for (int i = 0; i < n; i++) scanf("%d", &course_duration[i]);
    for (int j = 0; j < m; j++) scanf("%d", &room_capacity[j]);
    scanf("%d", &k);
    for (int e = 0; e < k; e++) {
        int u, v; scanf("%d %d", &u, &v);
        u--; v--;
        set_conflict(u, v);
        set_conflict(v, u);
    }
}

int main(int argc, char **argv) {
    rng_state = (uint32_t)time(NULL);
    if (argc >= 2) {
        input_file(argv[1]); 
    } else {
        input_manual();
    }

    aco();
    for (int i = 0; i < n; i++) {
        printf("%d %d %d\n", i + 1, best_slot_assignment[i], best_room_assignment[i]);
    }
    return 0;
}
