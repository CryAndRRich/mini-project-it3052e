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

#define restart_limit 1000
#define random_trials 100

static uint32_t rng_state;

static inline uint32_t fast_rand(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static inline double frand(void) {
    return (double)fast_rand() / (double)UINT32_MAX;
}

int n, m, k;
int course_duration[max_n];
int room_capacity[max_m];
uint64_t conflict_bits[max_n][w_words];

int room_timestamp[max_m][max_slot];
int room_usage[max_m][max_slot];

int slot_timestamp[max_slot][w_words];
uint64_t slot_bits[max_slot][w_words];

int best_slot_assignment[max_n];
int best_room_assignment[max_n];
int best_cost = inf;
int slot_limit;
int epoch_counter = 1;

int count_per_slot[max_slot + 1];
int current_max_slot;

static inline bool can_assign(int cid, int slot_index) {
    uint64_t *cb = conflict_bits[cid];
    int w_idx = (slot_index - 1);
    for (int w = 0; w < w_words; w++) {
        uint64_t used = (slot_timestamp[w_idx][w] == epoch_counter
                         ? slot_bits[w_idx][w] : 0ULL);
        if (cb[w] & used) return false;
    }
    return true;
}

static inline void apply_assign(int cid, int room, int slot) {
    room_timestamp[room][slot] = epoch_counter;
    room_usage[room][slot] = cid;
    int w = cid >> 6, b = cid & 63;
    slot_timestamp[slot][w] = epoch_counter;
    slot_bits[slot][w] |= (1ULL << b);

    int cnt_idx = slot + 1;
    count_per_slot[cnt_idx]++;
    if (cnt_idx > current_max_slot) current_max_slot = cnt_idx;
}

static inline void remove_assign(int cid, int room, int slot) {
    room_timestamp[room][slot] = -epoch_counter;
    int w = cid >> 6, b = cid & 63;
    if (slot_timestamp[slot][w] == epoch_counter) {
        slot_bits[slot][w] &= ~(1ULL << b);
    }
    int cnt_idx = slot + 1;
    if (--count_per_slot[cnt_idx] == 0 && cnt_idx == current_max_slot) {
        while (current_max_slot > 0 && count_per_slot[current_max_slot] == 0) {
            current_max_slot--;
        }
    }
}

static inline void clear_usage(void) {
    epoch_counter++;
    for (int i = 1; i <= slot_limit; i++) {
        count_per_slot[i] = 0;
    }
    current_max_slot = 0;
}

bool greedy_assignment(int order[]) {
    clear_usage();
    int tmp_slot[max_n], tmp_room[max_n];
    for (int i = 0; i < n; i++) {
        tmp_slot[i] = tmp_room[i] = 0;
    }

    int cap = best_cost - 1;
    if (cap > slot_limit) cap = slot_limit;

    for (int idx = 0; idx < n; idx++) {
        int cid = order[idx];
        bool placed = false;
        for (int r = 0; r < m && !placed; r++) {
            if (room_capacity[r] < course_duration[cid]) continue;
            for (int s = 0; s < cap; s++) {
                if (room_timestamp[r][s] == epoch_counter) continue;
                if (!can_assign(cid, s+1)) continue;
                tmp_slot[cid] = s+1;
                tmp_room[cid] = r+1;
                apply_assign(cid, r, s);
                placed = true;
                break;
            }
        }
        if (!placed) return false;
    }

    for (int i = 0; i < n; i++) {
        best_slot_assignment[i] = tmp_slot[i];
        best_room_assignment[i] = tmp_room[i];
    }
    best_cost = current_max_slot;
    return true;
}

void simulated_annealing(void) {
    int cur_slot[max_n], cur_room[max_n];
    for (int i = 0; i < n; i++) {
        cur_slot[i] = best_slot_assignment[i];
        cur_room[i] = best_room_assignment[i];
    }

    clear_usage();
    for (int i = 0; i < n; i++) {
        if (cur_slot[i] && cur_room[i]) {
            apply_assign(i, cur_room[i]-1, cur_slot[i]-1);
        }
    }
    int cur_cost = current_max_slot;

    double T = 100.0;
    double Tmin = 1e-6;
    double alpha= 0.999999;

    while (T > Tmin) {
        int max_cid = 0;
        for (int i = 1; i < n; i++)
            if (cur_slot[i] > cur_slot[max_cid])
                max_cid = i;

        int cid = (frand() < 0.5 ? max_cid : fast_rand() % n);

        int old_s = cur_slot[cid] - 1;
        int old_r = cur_room[cid] - 1;
        remove_assign(cid, old_r, old_s);

        bool found = false;
        int new_s, new_r;
        for (int t = 0; t < random_trials; t++) {
            new_s = fast_rand() % slot_limit;
            new_r = fast_rand() % m;
            if (room_capacity[new_r] < course_duration[cid]
             || room_timestamp[new_r][new_s] == epoch_counter
             || !can_assign(cid, new_s+1)) continue;
            found = true;
            break;
        }
        if (!found) {
            apply_assign(cid, old_r, old_s);
            T *= alpha;
            continue;
        }

        cur_slot[cid] = new_s+1;
        cur_room[cid] = new_r+1;
        apply_assign(cid, new_r, new_s);

        int new_cost = current_max_slot;
        int delta = new_cost - cur_cost;
        if (delta < 0 || frand() < exp(-delta/T)) {
            cur_cost = new_cost;
            if (cur_cost < best_cost) {
                best_cost = cur_cost;
                for (int i = 0; i < n; i++) {
                    best_slot_assignment[i] = cur_slot[i];
                    best_room_assignment[i] = cur_room[i];
                }
            }
        } else {
            remove_assign(cid, new_r, new_s);
            cur_slot[cid] = old_s+1;
            cur_room[cid] = old_r+1;
            apply_assign(cid, old_r, old_s);
        }
        T *= alpha;
    }
}

void local_search(void) {
    bool improved = true;
    while (improved) {
        improved = false;
        clear_usage();
        for (int i = 0; i < n; i++) {
            if (best_slot_assignment[i] && best_room_assignment[i]) {
                apply_assign(i, best_room_assignment[i]-1, best_slot_assignment[i]-1);
            }
        }

        for (int cid = 0; cid < n && !improved; cid++) {
            int old_s = best_slot_assignment[cid] - 1;
            int old_r = best_room_assignment[cid] - 1;
            for (int s = 0; s < old_s && !improved; s++) {
                for (int r = 0; r < m; r++) {
                    if (room_capacity[r] < course_duration[cid]
                     || room_timestamp[r][s] == epoch_counter
                     || !can_assign(cid, s+1)) { continue; }

                    remove_assign(cid, old_r, old_s);
                    apply_assign(cid, r, s);
                    int new_cost = current_max_slot;

                    if (new_cost < best_cost) {
                        best_cost = new_cost;
                        best_slot_assignment[cid] = s+1;
                        best_room_assignment[cid] = r+1;
                        improved = true;
                    } else {
                        remove_assign(cid, r, s);
                        apply_assign(cid, old_r, old_s);
                    }
                    if (improved) break;
                }
            }
        }
    }
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
    slot_limit = n;
    for (int i = 0; i < n; i++) { fscanf(f, "%d", &course_duration[i]); }
    for (int i = 0; i < m; i++) { fscanf(f, "%d", &room_capacity[i]); }
    fscanf(f, "%d", &k);
    for (int i = 0; i < k; i++) {
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
    slot_limit = n;
    for (int i = 0; i < n; i++) { scanf("%d", &course_duration[i]); }
    for (int i = 0; i < m; i++) { scanf("%d", &room_capacity[i]); }
    scanf("%d", &k);
    for (int i = 0; i < k; i++) {
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

    int order[max_n];
    for (int i = 0; i < n; i++) order[i] = i;

    for (int iter = 0; iter < restart_limit; iter++) {
        random_shuffle(order, n);
        if (!greedy_assignment(order)) continue;
        simulated_annealing();
        local_search();
    }

    for (int i = 0; i < n; i++) {
        printf("%d %d %d\n", i+1, best_slot_assignment[i], best_room_assignment[i]);
    }
    return 0;
}
