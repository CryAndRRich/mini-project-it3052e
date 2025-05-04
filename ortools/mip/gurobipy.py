import gurobipy as grb
import time

def solve_timetabling(N, M, d, c, conflict_pairs):
    max_days = N
    slots_per_day = 4
    S = max_days * slots_per_day

    model = grb.Model()

    x = {}
    for i in range(N):
        for s in range(S):
            for r in range(M):
                x[(i, s, r)] = model.addVar(vtype=grb.GRB.BINARY, name=f"x_{i}_{s}_{r}")
                if c[r] < d[i]:
                    model.addConstr(x[(i, s, r)] == 0)

    D = model.addVar(vtype=grb.GRB.INTEGER, name="D", lb=1, ub=S)

    model.update()

    for i in range(N):
        model.addConstr(grb.quicksum(x[(i, s, r)] for s in range(S) for r in range(M)) == 1)

    for s in range(S):
        for r in range(M):
            model.addConstr(grb.quicksum(x[(i, s, r)] for i in range(N)) <= 1)

    for (i, j) in conflict_pairs:
        for s in range(S):
            model.addConstr(grb.quicksum(x[(i, s, r)] + x[(j, s, r)] for r in range(M)) <= 1)

    for i in range(N):
        model.addConstr(grb.quicksum((s + 1) * x[(i, s, r)] for s in range(S) for r in range(M)) <= D)

    model.setObjective(D, grb.GRB.MINIMIZE)

    model.optimize()

    if model.status not in (grb.GRB.OPTIMAL, grb.GRB.SUBOPTIMAL, grb.GRB.ITERATION_LIMIT):  
        print("No solution found")
        return None, None

    slots = [None] * N
    rooms = [None] * N
    for i in range(N):
        for s in range(S):
            for r in range(M):
                if x[(i, s, r)].x > 0.5:
                    slots[i] = s + 1
                    rooms[i] = r + 1

    return slots, rooms


if __name__ == "__main__":
    input_path = "test/type_1/test_01.txt"

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