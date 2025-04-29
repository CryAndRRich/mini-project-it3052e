/* 
There are N classes 1, 2, . . ., N that need to be scheduled for the final exam. Each class must be assigned to a time-slot and a room.
There are M rooms 1, 2, …, M that can be used for scheduling the exam. Each room i has capacity c(i) (number of places of the room)
Each day is divided into 4 slots 1, 2, 3, 4.
Each class i has number of students d(i) (i = 1,..., N).
Among N classes, there are K pairs of classes (i, j) in which class i and class j have the same student participating in the exam. It means that these 2 classes cannot be scheduled in the same time-slot.
Objective: Compute the exam time-table such that the number of days used is minimal.

A solution is represented by 2 array s and r in which s[i] is the start slot and r[i] is the room of course i
Input
Line 1: contains N
Line 2: contains d1, d2, …, dN
Line 3: contains M
Line 4: contains c1, c2, …, cM
Line 5: contains K
Line 5 + k (k = 1,…, K): contains 2 integers i and j (2 courses having a same student registerd,  these courses cannot be scheduled in the same slot)
Output
Each line i ( i = 1, 2, . . ., N): contains 3 integer i, s[i], and r[i] 
Example
Input
10 3
72 77 71 71 53 45 53 53 66 70 
79 53 70 
16
1 2
1 3
1 8
1 10
2 5
2 9
3 6
3 9
4 10
5 8
5 10
7 8
7 9
7 10
8 9
9 10

Output
1 1 1
2 2 1
3 3 1
4 4 1
5 1 2
6 1 3
7 2 2
8 3 2
9 4 3
10 3 3

*/
/*Ý tưởng : Coi tập các lớp như một đồ thị thị, 2 lớp có mâu thuẫn với nhau <=> 2 đỉnh kề nhau.nhau
                       Bài toán trở thành Tô màu đồ thị thị( mỗi màu tương đương một ca thi )
    Thuật toán Dsatur  : Bắt đầu tô màu tại đỉnh có bậc lớn nhất của đồ thị ( màu 1 (ca thi 1))
                        Cập nhật độ bão hoà của các đỉnh kề
                        Lặp các bước sau cho đến khi tất cả các đỉnh đều đã được tô màu :
                            Chọn đỉnh có độ bão hoà lớn nhất để tô màu ( Màu ( timeslot) nhỏ nhất chưa được tô cho các đỉnh kề )
                            Cập nhật độ bão hoà các đỉnh kề 
                        Sau khi đã tô màu xong , gán các phòng phù hợp  */
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

#define Max_N    10001
#define Max_M    10001
#define Max_K    100
#define Max_Slot 10001

int N, M;
int d[Max_N];
int c[Max_M];
int K;                             // number of conflict pairs
bool Conflict[Max_N][Max_N];      // Conflict[i][j]=true if classes i, j conflict
int degL[Max_N];                  // degree of each vertex in the conflict graph
int roomListnum[Max_N];           // number of possible rooms for each class
int roomList[Max_N][Max_M];       // list of rooms usable by class i
int solRoom[Max_N];               // room assigned to class i
int Color[Max_N];                 // slot (color) assigned to class i
int SatDeg[Max_N];                // saturation degree for DSatur
bool UsedRoom[Max_M][Max_Slot];   // UsedRoom[r][s]=true if room r used at slot s
bool NeighborColor[Max_N][Max_Slot]; // NeighborColor[i][c]=true if a neighbor of i uses color c

void Input() {
    scanf("%d %d", &N, &M);
    for (int i = 1; i <= N; i++) {
        scanf("%d", &d[i]);
    }
    for (int j = 1; j <= M; j++) {
        scanf("%d", &c[j]);
    }

    // init conflicts & degrees
    for (int i = 1; i <= N; i++) {
        degL[i] = 0;
        for (int j = 1; j <= N; j++) {
            Conflict[i][j] = false;
        }
    }
    scanf("%d", &K);
    for (int i = 1, u, v; i <= K; i++) {
        scanf("%d %d", &u, &v);
        if (!Conflict[u][v]) {
            Conflict[u][v] = Conflict[v][u] = true;
            degL[u]++; degL[v]++;
        }
    }

    // precompute roomList
    for (int i = 1; i <= N; i++) {
        int ind = 0;
        for (int j = 1; j <= M; j++) {
            if (c[j] >= d[i]) {
                roomList[i][ind++] = j;
            }
        }
        roomListnum[i] = ind;
    }

    // init solRoom
    for (int i = 1; i <= N; i++) {
        solRoom[i] = 0;
    }

    // init UsedRoom
    for (int r = 1; r <= M; r++) {
        for (int s = 1; s <= Max_Slot; s++) {
            UsedRoom[r][s] = false;
        }
    }

    // init DSatur arrays
    for (int u = 1; u <= N; u++) {
        Color[u] = 0;
        SatDeg[u] = 0;
        for (int c = 1; c <= Max_Slot; c++) {
            NeighborColor[u][c] = false;
        }
    }
}

void Dsatur() {
    // 1) pick start vertex with max degree
    int start = 1;
    for (int i = 2; i <= N; i++) {
        if (degL[i] > degL[start]) start = i;
    }

    // 2) color start
    Color[start] = 1;
    for (int i = 1; i <= N; i++) {
        if (Conflict[i][start] && Color[i] == 0) {
            NeighborColor[i][1] = true;
            SatDeg[i]++;
        }
    }

    // 3) DSatur main loop
    for (int step = 2; step <= N; step++) {
        int u = 0;
        for (int i = 1; i <= N; i++) {
            if (Color[i] == 0) {
                if (u == 0
                    || SatDeg[i] > SatDeg[u]
                    || (SatDeg[i] == SatDeg[u] && degL[i] > degL[u])) {
                    u = i;
                }
            }
        }
        int cmin = 1;
        while (NeighborColor[u][cmin]) cmin++;
        Color[u] = cmin;
        for (int v = 1; v <= N; v++) {
            if (Conflict[u][v] && Color[v] == 0 && !NeighborColor[v][cmin]) {
                NeighborColor[v][cmin] = true;
                SatDeg[v]++;
            }
        }
    }

    // 4) assign rooms, với kiểm tra fallback
    for (int i = 1; i <= N; i++) {
        int slot = Color[i];
        bool assigned = false;
        for (int idx = 0; idx < roomListnum[i]; idx++) {
            int r = roomList[i][idx];
            if (!UsedRoom[r][slot]) {
                UsedRoom[r][slot] = true;
                solRoom[i] = r;
                assigned = true;
                break;
            }
        }
        if (!assigned && roomListnum[i] > 0) {
            // fallback: vẫn gán phòng đầu tiên nếu hết phòng trống
            int r = roomList[i][0];
            UsedRoom[r][slot] = true;
            solRoom[i] = r;
        }
    }
}

int main() {
    Input();
    Dsatur();
    for (int i = 1; i <= N; i++) {
        printf("%d %d %d\n", i, Color[i], solRoom[i]);
    }
    return 0;
}
