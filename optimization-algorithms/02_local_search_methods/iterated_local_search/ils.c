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

#define ils_iters 1000
#define perturb_k 25
#define random_trials 1000

static uint32_t rng_state;

static inline uint32_t fast_rand(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static int n, m, k;
static int course_duration[max_n];
static int room_capacity[max_m];

static uint64_t conflict_bits[max_n][w_words];
static uint64_t slot_bits[max_slot][w_words];

static int slot_assignment[max_n], room_assignment[max_n];
static int curr_slot[max_n], curr_room[max_n];
static int best_slot_assignment[max_n], best_room_assignment[max_n];
static int best_cost = inf;

static int room_ts[max_m][max_slot];
static int slot_ts[max_slot][w_words];
static int epoch_counter = 1;
static int count_per_slot[max_slot + 1];
static int cur_max_slot;

static inline void clear_usage(void) {
    epoch_counter++;
    cur_max_slot = 0;
    for (int i = 1; i <= max_slot; i++) {
        count_per_slot[i] = 0;
    }
}

static inline void build_usage_from_solution(void) {
    clear_usage();
    for (int cid = 0; cid < n; cid++) {
        int sl = slot_assignment[cid];
        int rm = room_assignment[cid] - 1;
        if (sl > 0 && rm >= 0) {
            int s = sl - 1;
            room_ts[rm][s] = epoch_counter;
            slot_ts[s][cid>>6] = epoch_counter;
            slot_bits[s][cid>>6] |= 1ULL << (cid & 63);
            int idx = s + 1;
            count_per_slot[idx]++;
            if (idx > cur_max_slot) cur_max_slot = idx;
        }
    }
}

static inline bool can_assign(int cid, int s) {
    int idx = s - 1;
    for (int w = 0; w < w_words; w++) {
        uint64_t sb = (slot_ts[idx][w] == epoch_counter ? slot_bits[idx][w] : 0ULL);
        if (conflict_bits[cid][w] & sb) return false;
    }
    return true;
}

static inline void apply_assign(int cid, int rm, int s) {
    room_ts[rm][s] = epoch_counter;
    slot_ts[s][cid>>6] = epoch_counter;
    slot_bits[s][cid>>6] |= 1ULL << (cid & 63);
    int idx = s + 1;
    count_per_slot[idx]++;
    if (idx > cur_max_slot) cur_max_slot = idx;
}

static inline void remove_assign(int cid, int rm, int s) {
    room_ts[rm][s] = -epoch_counter;
    if (slot_ts[s][cid>>6] == epoch_counter)
        slot_bits[s][cid>>6] &= ~(1ULL << (cid & 63));
    int idx = s + 1;
    if (--count_per_slot[idx] == 0 && idx == cur_max_slot) {
        while (cur_max_slot > 0 && count_per_slot[cur_max_slot] == 0)
            cur_max_slot--;
    }
}

static inline void random_valid_assignment(int order[]) {
    build_usage_from_solution();
    int limit = best_cost > 1 ? best_cost - 1 : max_slot;
    for (int i = 0; i < n; i++) {
        slot_assignment[i] = 0;
        room_assignment[i] = 0;
    }

    for (int idx = 0; idx < n; idx++) {
        int cid = order[idx];
        for (int t = 0; t < random_trials; t++) {
            int s = fast_rand() % limit;
            if (!can_assign(cid, s+1)) continue;
            int rm = fast_rand() % m;
            if (room_capacity[rm] < course_duration[cid]
             || room_ts[rm][s] == epoch_counter) continue;
            apply_assign(cid, rm, s);
            slot_assignment[cid] = s + 1;
            room_assignment[cid] = rm + 1;
            break;
        }
    }
    if (cur_max_slot < best_cost) best_cost = cur_max_slot;
}

static inline void local_search(void) {
    bool improved = true;
    while (improved) {
        improved = false;
        build_usage_from_solution();
        for (int cid = 0; cid < n && !improved; cid++) {
            int old_s = slot_assignment[cid];
            int old_rm = room_assignment[cid] - 1;
            if (old_s <= 1) continue;
            for (int ns = 1; ns < old_s && !improved; ns++) {
                if (!can_assign(cid, ns)) continue;
                for (int nr = 0; nr < m; nr++) {
                    if (room_capacity[nr] < course_duration[cid]
                     || room_ts[nr][ns-1] == epoch_counter) continue;
                    remove_assign(cid, old_rm, old_s-1);
                    apply_assign(cid, nr, ns-1);
                    slot_assignment[cid] = ns;
                    room_assignment[cid] = nr + 1;
                    improved = true;
                    break;
                }
            }
        }
    }
    if (cur_max_slot < best_cost) best_cost = cur_max_slot;
}

static inline void perturb(void) {
    for (int t = 0; t < perturb_k; t++) {
        int cid = fast_rand() % n;
        int old_s = slot_assignment[cid] - 1;
        int old_rm = room_assignment[cid] - 1;
        if (old_s >= 0 && old_rm >= 0) remove_assign(cid, old_rm, old_s);
        for (int r = 0; r < random_trials; r++) {
            int s = fast_rand() % max_slot;
            if (!can_assign(cid, s+1)) continue;
            int rm = fast_rand() % m;
            if (room_capacity[rm] < course_duration[cid]
             || room_ts[rm][s] == epoch_counter) continue;
            apply_assign(cid, rm, s);
            slot_assignment[cid] = s + 1;
            room_assignment[cid] = rm + 1;
            break;
        }
    }
}

static inline int compute_cost(void) {
    int cost = 0;
    for (int i = 0; i < n; i++)
        if (slot_assignment[i] > cost) cost = slot_assignment[i];
    return cost;
}

static inline void random_shuffle(int *arr, int len) {
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
    for (int i = 0; i < n; i++) fscanf(f, "%d", &course_duration[i]);
    for (int j = 0; j < m; j++) fscanf(f, "%d", &room_capacity[j]);
    fscanf(f, "%d", &k);
    for (int e = 0; e < k; e++) {
        int u, v; fscanf(f, "%d %d", &u, &v);
        u--; v--;
        conflict_bits[u][v>>6] |= 1ULL << (v & 63);
        conflict_bits[v][u>>6] |= 1ULL << (u & 63);
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

    int order_arr[max_n];
    for (int i = 0; i < n; i++) order_arr[i] = i;

    random_shuffle(order_arr, n);
    build_usage_from_solution();
    random_valid_assignment(order_arr);
    local_search();
    for (int i = 0; i < n; i++) {
        curr_slot[i] = slot_assignment[i];
        curr_room[i] = room_assignment[i];
    }
    best_cost = cur_max_slot;
    for (int i = 0; i < n; i++) {
        best_slot_assignment[i] = slot_assignment[i];
        best_room_assignment[i] = room_assignment[i];
    }

    for (int iter = 0; iter < ils_iters; iter++) {
        for (int i = 0; i < n; i++) {
            slot_assignment[i] = curr_slot[i];
            room_assignment[i] = curr_room[i];
        }
        build_usage_from_solution();
        perturb();
        local_search();
        for (int i = 0; i < n; i++) {
            curr_slot[i] = slot_assignment[i];
            curr_room[i] = room_assignment[i];
        }
        if (cur_max_slot < best_cost) {
            best_cost = cur_max_slot;
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
