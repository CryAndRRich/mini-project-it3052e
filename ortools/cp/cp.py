from ortools.sat.python import cp_model
import time
import numpy as np
import gc

def solve_timetabling(N, M, d, c, conflict_pairs):
    model = cp_model.CpModel()
    max_slots = N
    slot = [model.NewIntVar(1, max_slots, f"slot_{i}") for i in range(N)]
    room = [model.NewIntVar(1, M, f"room_{i}") for i in range(N)]

    for i, j in conflict_pairs:
        model.Add(slot[i] != slot[j])

    for i in range(N):
        for r in range(1, M+1):
            if c[r-1] < d[i]:
                model.Add(room[i] != r)

    for i in range(N):
        for j in range(i+1, N):
            b = model.NewBoolVar(f"same_slot_{i}_{j}")
            model.Add(slot[i] == slot[j]).OnlyEnforceIf(b)
            model.Add(slot[i] != slot[j]).OnlyEnforceIf(b.Not())
            model.Add(room[i] != room[j]).OnlyEnforceIf(b)

    max_slot_used = model.NewIntVar(1, max_slots, "max_slot_used")
    model.AddMaxEquality(max_slot_used, slot)
    model.Minimize(max_slot_used)

    solver = cp_model.CpSolver()
    solver.parameters.max_time_in_seconds = 300
    solver.parameters.num_search_workers = 8
    result = solver.Solve(model)

    if result in (cp_model.OPTIMAL, cp_model.FEASIBLE):
        slots = [solver.Value(slot[i]) for i in range(N)]
        rooms = [solver.Value(room[i]) for i in range(N)]
        return slots, rooms
    else:
        return None, None

def read_input(input_path):
    with open(input_path, "r") as f:
        data = f.read().split()
    it = iter(data)
    N = int(next(it))
    M = int(next(it))
    d = [int(next(it)) for _ in range(N)]
    c = [int(next(it)) for _ in range(M)]
    K = int(next(it))
    pairs = [(int(next(it)) - 1, int(next(it)) - 1) for _ in range(K)]
    return N, M, d, c, pairs

if __name__ == "__main__":
    input_path = "/content/test_28.txt"
    N, M, d, c, pairs = read_input(input_path)

    NUM_RUNS = 10
    WARMUP_RUNS = 2
    times = []

    print("Running warm-up...")
    for _ in range(WARMUP_RUNS):
        solve_timetabling(N, M, d, c, pairs)
        gc.collect()  

    print(f"Running {NUM_RUNS} iterations...")
    for i in range(NUM_RUNS):
        gc.collect()

        t_start = time.perf_counter()  
        slots, rooms = solve_timetabling(N, M, d, c, pairs)
        t_end = time.perf_counter()

        elapsed = t_end - t_start
        times.append(elapsed)
        print(f"Run {i+1}: {elapsed:.8f} seconds")

    mean_time = np.mean(times)

    print("\nThống kê thời gian chạy:")
    print(f"Thời gian trung bình: {mean_time:.8f} giây")
    if slots is not None:
        for i in range(N):
            print(i + 1, slots[i], rooms[i])
        print(f"Số slot tối đa: {max(slots)}")
    else:
        print("No solution found")
