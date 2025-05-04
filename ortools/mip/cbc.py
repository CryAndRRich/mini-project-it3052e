from ortools.linear_solver import pywraplp
import time

def solve_timetabling(N, M, d, c, conflict_pairs):
    max_days = N
    slots_per_day = 4
    S = max_days * slots_per_day 

    solver = pywraplp.Solver.CreateSolver("CBC")
    solver.SetTimeLimit(300_000)

    x = {}
    for i in range(N):
        for s in range(S):
            for r in range(M):
                if c[r] >= d[i]:
                    x[(i, s, r)] = solver.IntVar(0, 1, f"x_{i}_{s}_{r}")

    D = solver.IntVar(1, S, "D")

    for i in range(N):
        solver.Add(sum(var for (ii, _, _), var in x.items() if ii == i) == 1)

    for s in range(S):
        for r in range(M):
            coll = [var for (i2, s2, r2), var in x.items() if s2 == s and r2 == r]
            if len(coll) > 1:
                solver.Add(sum(coll) <= 1)

    for (i, j) in conflict_pairs:
        for s in range(S):
            lhs = []
            for r in range(M):
                if (i, s, r) in x: lhs.append(x[(i, s, r)])
                if (j, s, r) in x: lhs.append(x[(j, s, r)])
            if lhs:
                solver.Add(sum(lhs) <= 1)

    for i in range(N):
        expr = solver.Sum((s + 1) * x[(i, s, r)]
                          for (ii, s, r) in x if ii == i)
        solver.Add(expr <= D)

    solver.Minimize(D)

    status = solver.Solve()

    if status not in (pywraplp.Solver.OPTIMAL, pywraplp.Solver.FEASIBLE):
        return None, None

    slots = [None] * N
    rooms = [None] * N
    for (i, s, r), var in x.items():
        if var.solution_value() > 0.5:
            slots[i] = s + 1
            rooms[i] = r + 1

    return slots, rooms

if __name__ == "__main__":
    input_path = "test/type_1/test_14.txt"

    with open(input_path, "r") as f:
        data = f.read().split()
    it = iter(data)

    N = int(next(it))
    M = int(next(it))
    d = [int(next(it)) for _ in range(N)]
    c = [int(next(it)) for _ in range(M)]
    K = int(next(it))
    pairs = [(int(next(it)) - 1, int(next(it)) - 1) for _ in range(K)]

    t_start = time.time()
    slots, rooms = solve_timetabling(N, M, d, c, pairs)
    t_end = time.time()
    elapsed = t_end - t_start

    if slots is not None:
        for i in range(N):
            print(i + 1, slots[i], rooms[i])
        print(max(slots))
    else:
        print("No solution found")
        
    print(f"Runtime: {elapsed:.8f} seconds")