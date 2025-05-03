#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define max_n 3000
#define max_m 20
#define max_slot (max_n * 4)
#define w_words ((max_n + 63) / 64)

int n, m, k;
int course_duration[max_n];
int room_capacity[max_m];

uint64_t conflict_bits[max_n][w_words];

int deg_list[max_n];
int rooms_count[max_n];
int rooms_list[max_n][max_m];
int sol_room[max_n];

int color_slot[max_n];
int sat_deg[max_n];
bool used_room[max_m][max_slot];
bool neighbor_color[max_n][max_slot];

static void input_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen"); exit(EXIT_FAILURE); }
    fscanf(f, "%d %d", &n, &m);
    for (int i = 0; i < n; i++) fscanf(f, "%d", &course_duration[i]);
    for (int j = 0; j < m; j++) fscanf(f, "%d", &room_capacity[j]);
    fscanf(f, "%d", &k);

    for (int i = 0; i < n; i++) {
        deg_list[i] = 0;
        for (int w = 0; w < w_words; w++)
            conflict_bits[i][w] = 0ULL;
    }
    for (int e = 0; e < k; e++) {
        int u, v;
        fscanf(f, "%d %d", &u, &v);
        u--; v--;
        conflict_bits[u][v>>6] |= 1ULL << (v & 63);
        conflict_bits[v][u>>6] |= 1ULL << (u & 63);
        deg_list[u]++;
        deg_list[v]++;
    }
    for (int i = 0; i < n; i++) {
        int cnt = 0;
        for (int j = 0; j < m; j++) {
            if (room_capacity[j] >= course_duration[i])
                rooms_list[i][cnt++] = j;
        }
        rooms_count[i] = cnt;
    }
    fclose(f);
}

static void input_manual(void) {
    scanf("%d %d", &n, &m);
    for (int i = 0; i < n; i++) scanf("%d", &course_duration[i]);
    for (int j = 0; j < m; j++) scanf("%d", &room_capacity[j]);
    scanf("%d", &k);

    for (int i = 0; i < n; i++) {
        deg_list[i] = 0;
        for (int w = 0; w < w_words; w++)
            conflict_bits[i][w] = 0ULL;
    }
    for (int e = 0; e < k; e++) {
        int u, v;
        scanf("%d %d", &u, &v);
        u--; v--;
        conflict_bits[u][v>>6] |= 1ULL << (v & 63);
        conflict_bits[v][u>>6] |= 1ULL << (u & 63);
        deg_list[u]++;
        deg_list[v]++;
    }
    for (int i = 0; i < n; i++) {
        int cnt = 0;
        for (int j = 0; j < m; j++) {
            if (room_capacity[j] >= course_duration[i])
                rooms_list[i][cnt++] = j;
        }
        rooms_count[i] = cnt;
    }
}

static inline void dsatur(void) {
    for (int r = 0; r < m; r++)
        for (int s = 0; s < max_slot; s++)
            used_room[r][s] = false;

    for (int i = 0; i < n; i++) {
        color_slot[i] = -1;
        sat_deg[i] = 0;
        for (int s = 0; s < max_slot; s++)
            neighbor_color[i][s] = false;
    }

    int start = 0;
    for (int i = 1; i < n; i++)
        if (deg_list[i] > deg_list[start])
            start = i;

    color_slot[start] = 0;
    for (int v = 0; v < n; v++) {
        if ((conflict_bits[start][v >> 6] >> (v & 63)) & 1ULL) {
            neighbor_color[v][0] = true;
            sat_deg[v]++;
        }
    }

    for (int step = 1; step < n; step++) {
        int u = -1;
        for (int i = 0; i < n; i++) {
            if (color_slot[i] < 0 && (u < 0 || sat_deg[i] > sat_deg[u] || (sat_deg[i] == sat_deg[u] && deg_list[i] > deg_list[u]))) {
                u = i;
            }
        }
        int cmin = 0;
        while (neighbor_color[u][cmin]) cmin++;
        color_slot[u] = cmin;
        for (int v = 0; v < n; v++) {
            if (color_slot[v] < 0 && ((conflict_bits[u][v >> 6] >> (v & 63)) & 1ULL) && !neighbor_color[v][cmin]) {
                neighbor_color[v][cmin] = true;
                sat_deg[v]++;
            }
        }
    }

    for (int i = 0; i < n; i++) {
        int base_slot = color_slot[i];
        sol_room[i] = -1;

        for (int idx = 0; idx < rooms_count[i]; idx++) {
            int r = rooms_list[i][idx];
            if (!used_room[r][base_slot]) {
                sol_room[i] = r + 1;
                used_room[r][base_slot] = true;
                break;
            }
        }

        if (sol_room[i] < 0) {
            for (int s = 0; s < max_slot && sol_room[i] < 0; s++) {
                bool ok_slot = true;
                for (int w = 0; w < w_words && ok_slot; w++) {
                    uint64_t bits = conflict_bits[i][w];
                    while (bits) {
                        int b = __builtin_ctzll(bits);
                        bits &= bits - 1;
                        int j = (w << 6) + b;
                        if (j < n && color_slot[j] == s) {
                            ok_slot = false;
                            break;
                        }
                    }
                }
                if (!ok_slot) continue;

                for (int idx = 0; idx < rooms_count[i]; idx++) {
                    int r = rooms_list[i][idx];
                    if (!used_room[r][s]) {
                        color_slot[i] = s;
                        sol_room[i] = r + 1;
                        used_room[r][s] = true;
                        break;
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc >= 2) {
        input_file(argv[1]);
    } else {
        input_manual();
    }

    dsatur();

    for (int i = 0; i < n; i++) {
        printf("%d %d %d\n", i + 1, color_slot[i] + 1, sol_room[i]);
    }
    return 0;
}
