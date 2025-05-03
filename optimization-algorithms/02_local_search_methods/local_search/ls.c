#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <math.h>

#define max_n 3000
#define max_m 20
#define max_slot (max_n * 4)
#define inf (max_slot + 1)
#define w_words ((max_n + 63) / 64)

#define restart_limit 500

uint32_t rng_state;

static inline uint32_t fast_rand(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static inline void random_shuffle(int *arr, int len) {
    for (int i = len - 1; i > 0; i--) {
        int j = fast_rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
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

static inline void clear_room_usage(void) {
    for (int r = 0; r < m; r++)
        for (int s = 0; s < max_slot; s++)
            room_usage[r][s] = -1;
    for (int s = 0; s < max_slot; s++)
        for (int w = 0; w < w_words; w++)
            slot_bits[s][w] = 0ULL;
}

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

static inline bool greedy_assignment(void) {
    clear_room_usage();
    for (int i = 0; i < n; i++) {
        slot_assignment[i] = 0;
        room_assignment[i] = 0;
    }

    int slot_limit = best_cost - 1;
    if (slot_limit > max_slot) slot_limit = max_slot;

    for (int idx = 0; idx < n; idx++) {
        int cid = order[idx];
        bool assigned = false;

        for (int r = 0; r < m && !assigned; r++) {
            if (room_capacity[r] < course_duration[cid]) continue;
            for (int s = 0; s < slot_limit; s++) {
                if (room_usage[r][s] != -1) continue;
                if (!can_assign(cid, s+1)) continue;
                slot_assignment[cid] = s+1;
                room_assignment[cid] = r+1;
                apply_assign(cid, r, s);
                assigned = true;
                break;
            }
        }

        if (!assigned) return false;
    }
    return true;
}

static inline bool random_valid_assignment(void) {
    clear_room_usage();
    for (int i = 0; i < n; i++) {
        slot_assignment[i] = 0;
        room_assignment[i] = 0;
    }

    for (int idx = 0; idx < n; idx++) {
        int cid = order[idx];
        bool assigned = false;

        for (int trial = 0; trial < 10000 && !assigned; trial++) {
            int s = fast_rand() % max_slot;
            for (int r = 0; r < m && !assigned; r++) {
                if (room_capacity[r] < course_duration[cid]) continue;
                if (room_usage[r][s] != -1) continue;
                if (!can_assign(cid, s+1)) continue;
                slot_assignment[cid] = s+1;
                room_assignment[cid] = r+1;
                apply_assign(cid, r, s);
                assigned = true;
            }
        }

        if (!assigned) return false;
    }
    return true;
}

static inline void local_search(void) {
    bool improved = true;
    int iter = 0;
    while (improved && iter++ < 10000) {
        improved = false;
        for (int cid = 0; cid < n; cid++) {
            int old_s = slot_assignment[cid];
            int old_r = room_assignment[cid] - 1;
            if (old_s <= 1) continue;
            for (int new_s = 1; new_s < old_s && !improved; new_s++) {
                for (int r = 0; r < m; r++) {
                    if (room_capacity[r] < course_duration[cid]) continue;
                    if (room_usage[r][new_s-1] != -1) continue;
                    if (!can_assign(cid, new_s)) continue;
                    room_usage[old_r][old_s-1] = -1;
                    slot_bits[old_s-1][cid>>6] &= ~(1ULL << (cid & 63));
                    slot_assignment[cid] = new_s;
                    room_assignment[cid] = r+1;
                    apply_assign(cid, r, new_s-1);
                    improved = true;
                    break;
                }
            }
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

void input_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen"); exit(EXIT_FAILURE); }
    fscanf(f, "%d %d", &n, &m);
    for (int i = 0; i < n; i++)
        fscanf(f, "%d", &course_duration[i]);
    for (int j = 0; j < m; j++)
        fscanf(f, "%d", &room_capacity[j]);
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
    for (int i = 0; i < n; i++)
        scanf("%d", &course_duration[i]);
    for (int j = 0; j < m; j++)
        scanf("%d", &room_capacity[j]);
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

    for (int iter = 0; iter < restart_limit; iter++) {
        random_shuffle(order, n);
        bool ok;
        if (fast_rand() & 1)
            ok = greedy_assignment();
        else
            ok = random_valid_assignment();
        if (!ok) continue;

        local_search();
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
