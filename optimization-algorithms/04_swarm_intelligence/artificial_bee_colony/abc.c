#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>

#define max_n 3000
#define max_m 20
#define w_words ((max_n + 63) / 64)

#define max_bee 200
#define max_iter 100
#define max_trial 20

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

int sol_slot[max_n];
int sol_room[max_n];

int bee_slot[max_bee][max_n];
int bee_room[max_bee][max_n];

int trial_count[max_bee];
double fitness[max_bee];
double prob_select[max_bee];
double best_fitness;

int best_slot_arr[max_n];
int best_room_arr[max_n];

static inline int compute_penalty(const int slots[], const int rooms[]) {
    int pen = 0;
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j < i; j++) {
            if (conflict_bits[i][j>>6] & (1ULL << (j & 63))) {
                if (slots[i] == slots[j]) pen++;
            } else if (slots[i] == slots[j] && rooms[i] == rooms[j]) {
                pen++;
            }
        }
        if (room_capacity[rooms[i]] < course_duration[i]) pen++;
    }
    return pen;
}

static inline void init_colony(void) {
    int max_slots = n;
    for (int b = 0; b < max_bee; b++) {
        trial_count[b] = 0;
        for (int i = 1; i <= n; i++) {
            bee_slot[b][i] = (fast_rand() % max_slots) + 1;
            bee_room[b][i] = (fast_rand() % m) + 1;
        }
    }
}

static inline void compute_fitness(void) {
    best_fitness = -1.0;
    for (int b = 0; b < max_bee; b++) {
        int pen = compute_penalty(bee_slot[b], bee_room[b]);
        fitness[b] = 1.0 / (1.0 + pen);
        if (fitness[b] > best_fitness) {
            best_fitness = fitness[b];
            for (int i = 1; i <= n; i++) {
                best_slot_arr[i] = bee_slot[b][i];
                best_room_arr[i] = bee_room[b][i];
            }
        }
    }
}

static inline void employed_bees(void) {
    for (int b = 0; b < max_bee; b++) {
        int tmp_slot[max_n];
        int tmp_room[max_n];
        for (int i = 1; i <= n; i++) {
            tmp_slot[i] = bee_slot[b][i];
            tmp_room[i] = bee_room[b][i];
        }
        int i = (fast_rand() % n) + 1;
        if (fast_rand() & 1)
            tmp_slot[i] = (fast_rand() % n) + 1;
        else
            tmp_room[i] = (fast_rand() % m) + 1;
        int new_pen = compute_penalty(tmp_slot, tmp_room);
        int old_pen = compute_penalty(bee_slot[b], bee_room[b]);
        if (new_pen < old_pen) {
            for (int j = 1; j <= n; j++) {
                bee_slot[b][j] = tmp_slot[j];
                bee_room[b][j] = tmp_room[j];
            }
            trial_count[b] = 0;
        } else {
            trial_count[b]++;
        }
    }
}

static inline void calculate_probabilities(void) {
    double sum = 0.0;
    for (int b = 0; b < max_bee; b++) sum += fitness[b];
    for (int b = 0; b < max_bee; b++) prob_select[b] = fitness[b] / sum;
}

static inline void onlooker_bees(void) {
    calculate_probabilities();
    int count = 0;
    int idx = 0;
    while (count < max_bee) {
        if ((double)fast_rand() / UINT32_MAX < prob_select[idx]) {
            int tmp_slot[max_n];
            int tmp_room[max_n];
            for (int i = 1; i <= n; i++) {
                tmp_slot[i] = bee_slot[idx][i];
                tmp_room[i] = bee_room[idx][i];
            }
            int i = (fast_rand() % n) + 1;
            if (fast_rand() & 1)
                tmp_slot[i] = (fast_rand() % n) + 1;
            else
                tmp_room[i] = (fast_rand() % m) + 1;
            int new_pen = compute_penalty(tmp_slot, tmp_room);
            int old_pen = compute_penalty(bee_slot[idx], bee_room[idx]);
            if (new_pen < old_pen) {
                for (int j = 1; j <= n; j++) {
                    bee_slot[idx][j] = tmp_slot[j];
                    bee_room[idx][j] = tmp_room[j];
                }
                trial_count[idx] = 0;
            } else {
                trial_count[idx]++;
            }
            count++;
        }
        idx = (idx + 1) % max_bee;
    }
}

static inline void scout_bees(void) {
    for (int b = 0; b < max_bee; b++) {
        if (trial_count[b] > max_trial) {
            for (int i = 1; i <= n; i++) {
                bee_slot[b][i] = (fast_rand() % n) + 1;
                bee_room[b][i] = (fast_rand() % m) + 1;
            }
            trial_count[b] = 0;
        }
    }
}

static inline void abc(void) {
    init_colony();
    compute_fitness();
    for (int iter = 0; iter < max_iter; iter++) {
        employed_bees();
        compute_fitness();
        onlooker_bees();
        compute_fitness();
        scout_bees();
        compute_fitness();
    }
    for (int i = 1; i <= n; i++) {
        sol_slot[i] = best_slot_arr[i];
        sol_room[i] = best_room_arr[i];
    }
}

static inline void random_shuffle(int *arr, int len) {
    for (int i = len - 1; i > 0; i--) {
        int j = fast_rand() % (i + 1);
        int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }
}

void input_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen"); exit(EXIT_FAILURE); }
    fscanf(f, "%d %d", &n, &m);
    for (int i = 1; i <= n; i++) fscanf(f, "%d", &course_duration[i]);
    for (int j = 1; j <= m; j++) fscanf(f, "%d", &room_capacity[j]);
    fscanf(f, "%d", &k);
    for (int i = 0; i < max_n; i++)
        for (int w = 0; w < w_words; w++)
            conflict_bits[i][w] = 0ULL;
    for (int e = 0; e < k; e++) {
        int u, v;
        fscanf(f, "%d %d", &u, &v);
        conflict_bits[u][v>>6] |= 1ULL << (v & 63);
        conflict_bits[v][u>>6] |= 1ULL << (u & 63);
    }
    fclose(f);
}

void input_manual(void) {
    scanf("%d %d", &n, &m);
    for (int i = 1; i <= n; i++) scanf("%d", &course_duration[i]);
    for (int j = 1; j <= m; j++) scanf("%d", &room_capacity[j]);
    scanf("%d", &k);
    for (int i = 0; i < max_n; i++)
        for (int w = 0; w < w_words; w++)
            conflict_bits[i][w] = 0ULL;
    for (int e = 0; e < k; e++) {
        int u, v;
        scanf("%d %d", &u, &v);
        conflict_bits[u][v>>6] |= 1ULL << (v & 63);
        conflict_bits[v][u>>6] |= 1ULL << (u & 63);
    }
}

int main(int argc, char **argv) {
    rng_state = (uint32_t)time(NULL);
    if (argc >= 2) {
        input_file(argv[1]); 
    } else {
        input_manual();
    }

    abc();
    for (int i = 1; i <= n; i++) {
        printf("%d %d %d\n", i, sol_slot[i], sol_room[i]);
    }
    return 0;
}
