#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>

#define max_n 3000
#define max_m 20
#define max_slot (max_n * 4)
#define inf (max_slot + 1)
#define w_words ((max_n + 63) / 64)

#define random_iters 5000

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
uint64_t slot_bits[max_slot][w_words];

int slot_assignment[max_n];
int room_assignment[max_n];
int best_slot_assignment[max_n];
int best_room_assignment[max_n];
int best_cost = inf;

int room_usage[max_m][max_slot];
int order[max_n];

static inline bool can_assign(int cid, int s) {
    int idx = s - 1;
    for (int w = 0; w < w_words; w++) {
        if (conflict_bits[cid][w] & slot_bits[idx][w])
            return false;
    }
    return true;
}

static inline void apply_assign(int cid, int r, int s) {
    room_usage[r][s] = cid;
    int w = cid >> 6, b = cid & 63;
    slot_bits[s][w] |= (1ULL << b);
}

static inline void greedy_assign(void) {
    for (int i = 0; i < n; i++) {
        slot_assignment[i] = 0;
        room_assignment[i] = 0;
    }
    for (int r = 0; r < m; r++)
        for (int s = 0; s < max_slot; s++)
            room_usage[r][s] = -1;
    for (int s = 0; s < max_slot; s++)
        for (int w = 0; w < w_words; w++)
            slot_bits[s][w] = 0;

    for (int idx = 0; idx < n; idx++) {
        int cid = order[idx];
        int best_slot = inf;
        int best_room = -1;

        for (int r = 0; r < m; r++) {
            if (room_capacity[r] < course_duration[cid]) continue;

            for (int s = 0; s < max_slot; s++) {
                if (room_usage[r][s] != -1) continue;

                bool ok = true;
                for (int w = 0; w < w_words; w++) {
                    if (conflict_bits[cid][w] & slot_bits[s][w]) {
                        ok = false;
                        break;
                    }
                }
                if (!ok) continue;

                if (s + 1 < best_slot) {
                    best_slot = s + 1;
                    best_room = r;
                }
                break;
            }
        }

        if (best_room >= 0) {
            slot_assignment[cid] = best_slot;
            room_assignment[cid] = best_room + 1;
            room_usage[best_room][best_slot - 1] = cid;
            int w = cid >> 6, b = cid & 63;
            slot_bits[best_slot - 1][w] |= (1ULL << b);
        } else {
            slot_assignment[cid] = inf;
            room_assignment[cid] = 1;
        }
    }
}

static inline int compute_cost(void) {
    int cost = 0;
    for (int i = 0; i < n; i++)
        if (slot_assignment[i] > cost)
            cost = slot_assignment[i];
    return cost;
}

void random_shuffle(int *arr, int len) {
    for (int i = len - 1; i > 0; i--) {
        int j = fast_rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

void input_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen"); exit(EXIT_FAILURE); }
    fscanf(f, "%d %d", &n, &m);
    for (int i = 0; i < n; i++) { fscanf(f, "%d", &course_duration[i]); }
    for (int j = 0; j < m; j++) { fscanf(f, "%d", &room_capacity[j]); }
    fscanf(f, "%d", &k);
    for (int e = 0; e < k; e++) {
        int u, v;
        fscanf(f, "%d %d", &u, &v);
        u--; v--;
        conflict_bits[u][v>>6] |= 1ULL << (v & 63);
        conflict_bits[v][u>>6] |= 1ULL << (u & 63);
    }
    fclose(f);
}

void input_manual(void) {
    scanf("%d %d", &n, &m);
    for (int i = 0; i < n; i++) { scanf("%d", &course_duration[i]); }
    for (int j = 0; j < m; j++) { scanf("%d", &room_capacity[j]); }
    scanf("%d", &k);
    for (int e = 0; e < k; e++) {
        int u, v;
        scanf("%d %d", &u, &v);
        u--; v--;
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

    for (int i = 0; i < n; i++) order[i] = i;

    for (int iter = 0; iter < random_iters; iter++) {
        random_shuffle(order, n);
        greedy_assign();
        int cost = compute_cost();
        if (cost < best_cost) {
            best_cost = cost;
            for (int i = 0; i < n; i++) {
                best_slot_assignment[i] = slot_assignment[i];
                best_room_assignment[i] = room_assignment[i];
            }
        }
    }

    for (int i = 0; i < n; i++) {
        printf("%d %d %d\n", i+1, best_slot_assignment[i], best_room_assignment[i]);
    }
    return 0;
}
