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

#define restart_limit 100
#define tabu_tenure 20
#define max_iter 2000
#define no_improve_lim 50
#define random_trials 100

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
int room_usage[max_m][max_slot];
int slot_count[max_slot+1];

int best_slot_assignment[max_n];
int best_room_assignment[max_n];
int best_cost = inf;

typedef struct { 
    int cls; 
    int slot; 
} tabu_move;
static tabu_move tabu_list[tabu_tenure];
int tabu_head = 0;

int rooms_for_class[max_n][max_m];
int rooms_count[max_n];

static inline bool can_assign(int cls, int s) {
    int idx = s - 1;
    uint64_t *cb = conflict_bits[cls];
    uint64_t *sb = slot_bits[idx];
    for (int w = 0; w < w_words; w++) {
        if (cb[w] & sb[w]) return false;
    }
    return true;
}

static inline void clear_usage(void) {
    for (int i = 0; i < m; i++) {
        for (int s = 0; s < max_slot; s++) room_usage[i][s] = -1;
    }
    for (int s = 0; s < max_slot; s++) {
        for (int w = 0; w < w_words; w++) slot_bits[s][w] = 0ULL;
        slot_count[s+1] = 0;
    }
}

static inline void add_tabu(int cls, int s) {
    tabu_list[tabu_head] = (tabu_move){cls, s};
    tabu_head = (tabu_head + 1) % tabu_tenure;
}

static inline bool is_tabu(int cls, int s) {
    for (int i = 0; i < tabu_tenure; i++) {
        if (tabu_list[i].cls == cls && tabu_list[i].slot == s) return true;
    }
    return false;
}

static inline int find_second_max_slot(int current_cost) {
    for (int s = current_cost - 1; s >= 1; s--) {
        if (slot_count[s] > 0) return s;
    }
    return 0;
}

static inline bool greedy_assignment(int order[]) {
    clear_usage();
    for (int i = 0; i < n; i++) slot_assignment[i] = room_assignment[i] = 0;
    int limit = best_cost < max_slot ? best_cost - 1 : max_slot;
    for (int idx = 0; idx < n; idx++) {
        int cls = order[idx];
        bool done = false;
        for (int ri = 0; ri < rooms_count[cls] && !done; ri++) {
            int rm = rooms_for_class[cls][ri];
            for (int s = 1; s <= limit; s++) {
                if (room_usage[rm][s-1] != -1) continue;
                if (!can_assign(cls, s)) continue;
                slot_assignment[cls] = s;
                room_assignment[cls] = rm + 1;
                room_usage[rm][s-1] = cls;
                slot_bits[s-1][cls>>6] |= 1ULL << (cls & 63);
                slot_count[s]++;
                done = true;
                break;
            }
        }
        if (!done) return false;
    }
    return true;
}

static inline bool random_valid_assignment(int order[]) {
    clear_usage();
    for (int i = 0; i < n; i++) slot_assignment[i] = room_assignment[i] = 0;
    for (int idx = 0; idx < n; idx++) {
        int cls = order[idx];
        bool placed = false;
        for (int t = 0; t < random_trials && !placed; t++) {
            int s = fast_rand() % max_slot;
            for (int ri = 0; ri < rooms_count[cls] && !placed; ri++) {
                int rm = rooms_for_class[cls][ri];
                if (room_usage[rm][s] != -1) continue;
                if (!can_assign(cls, s+1)) continue;
                slot_assignment[cls] = s + 1;
                room_assignment[cls] = rm + 1;
                room_usage[rm][s] = cls;
                slot_bits[s][cls>>6] |= 1ULL << (cls & 63);
                slot_count[s+1]++;
                placed = true;
            }
        }
        if (!placed) return false;
    }
    return true;
}

static inline int compute_cost(void) {
    int cost = 0;
    for (int i = 0; i < n; i++) {
        if (slot_assignment[i] > cost) cost = slot_assignment[i];
    }
    return cost;
}

static inline void tabu_search(int order[]) {
    clear_usage();
    bool ok = (fast_rand() & 1) ? greedy_assignment(order) : random_valid_assignment(order);
    if (!ok) return;
    int current_cost = compute_cost();
    best_cost = current_cost;
    for (int i = 0; i < n; i++) {
        best_slot_assignment[i] = slot_assignment[i];
        best_room_assignment[i] = room_assignment[i];
    }

    int iter = 0, no_improve = 0;
    while (iter++ < max_iter && no_improve < no_improve_lim) {
        int top_list[max_n], top_count = 0;
        for (int i = 0; i < n; i++) {
            if (slot_assignment[i] >= current_cost - 1) top_list[top_count++] = i;
        }
        int best_nb_cost = inf, move_cls=-1, move_slot=-1, move_rm=-1;
        for (int ti = 0; ti < top_count; ti++) {
            int cls = top_list[ti];
            int old_s = slot_assignment[cls];
            for (int s = 1; s < best_cost; s++) {
                if (s == old_s) continue;
                if (!can_assign(cls, s)) continue;
                for (int ri = 0; ri < rooms_count[cls]; ri++) {
                    int rm = rooms_for_class[cls][ri];
                    if (room_usage[rm][s-1] != -1) continue;
                    if (is_tabu(cls, s) && current_cost >= best_cost) continue;
                    int new_cost = current_cost;
                    if (s > current_cost) new_cost = s;
                    else if (old_s == current_cost && slot_count[current_cost] == 1)
                        new_cost = find_second_max_slot(current_cost);
                    if (new_cost < best_nb_cost) {
                        best_nb_cost = new_cost;
                        move_cls = cls;
                        move_slot = s;
                        move_rm = rm;
                    }
                }
            }
        }
        if (move_cls < 0) break;
        int old_s = slot_assignment[move_cls];
        int old_rm = room_assignment[move_cls] - 1;
        room_usage[old_rm][old_s-1] = -1;
        slot_bits[old_s-1][move_cls>>6] &= ~(1ULL << (move_cls & 63));
        slot_count[old_s]--;
        slot_assignment[move_cls] = move_slot;
        room_assignment[move_cls] = move_rm + 1;
        room_usage[move_rm][move_slot-1] = move_cls;
        slot_bits[move_slot-1][move_cls>>6] |= 1ULL << (move_cls & 63);
        slot_count[move_slot]++;
        add_tabu(move_cls, old_s);
        current_cost = best_nb_cost;
        if (current_cost < best_cost) {
            best_cost = current_cost;
            for (int i = 0; i < n; i++) {
                best_slot_assignment[i] = slot_assignment[i];
                best_room_assignment[i] = room_assignment[i];
            }
            no_improve = 0;
        } else {
            no_improve++;
        }
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

    for (int i = 0; i < n; i++) {
        int cnt = 0;
        for (int j = 0; j < m; j++) {
            if (room_capacity[j] >= course_duration[i]) {
                rooms_for_class[i][cnt++] = j;
            }
        }
        rooms_count[i] = cnt;
    }
    int order[max_n];
    for (int i = 0; i < n; i++) order[i] = i;
    for (int it = 0; it < restart_limit; it++) {
        random_shuffle(order, n);
        tabu_search(order);
    }
    for (int i = 0; i < n; i++) {
        printf("%d %d %d\n", i+1, best_slot_assignment[i], best_room_assignment[i]);
    }
    return 0;
}
