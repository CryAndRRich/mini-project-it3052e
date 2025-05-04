from docplex.cp.model import CpoModel  
import time  

def solve_timetabling(N, M, d, c, conflict_pairs):
    mdl = CpoModel(name="ExamScheduling")  
    max_slots = N  
    slot = [mdl.integer_var(1, max_slots, f"slot{i}") for i in range(N)]  
    room = [mdl.integer_var(1, M, f"room{i}") for i in range(N)]  
    D = mdl.integer_var(1, max_slots, "D")  

    for i, j in conflict_pairs:
        mdl.add(slot[i] != slot[j])

    for i in range(N):
        for r in range(1, M+1):  
            if d[i] > c[r-1]:
                mdl.add(room[i] != r)

    for i in range(N):
        for j in range(i+1, N):
            mdl.add((slot[i] != slot[j]) | (room[i] != room[j]))

    for i in range(N):
        mdl.add(slot[i] <= D)

    mdl.minimize(D)

    start = time.perf_counter()  
    sol = mdl.solve()  
    end = time.perf_counter() 
    print(f"Runtime: {end-start:.8f} seconds")

    if sol is None:  
        return None, None
    
    slots = [int(sol.get_value(slot[i])) for i in range(N)]  
    rooms = [int(sol.get_value(room[i])) for i in range(N)]  
    return slots, rooms  

if __name__ == "__main__":
    input_path = "test/type_2/test_21.txt"

    with open(input_path, "r") as f:
        data = f.read().split()
    it = iter(data)

    N = int(next(it)); 
    M = int(next(it)) 
    d = [int(next(it)) for _ in range(N)]  
    c = [int(next(it)) for _ in range(M)]  
    K = int(next(it))  
    pairs = []  
    for _ in range(K):
        u = int(next(it)) - 1  
        v = int(next(it)) - 1
        pairs.append((u, v))  

    slots, rooms = solve_timetabling(N, M, d, c, pairs)  
    
    if slots is not None:  
        for i in range(N):  
            print(i+1, slots[i], rooms[i])  
    else:
        print("No solution found")  
