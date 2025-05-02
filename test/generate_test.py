import random
import sys
from typing import Tuple

def generate(N: int, M: int, K: int, d_range: Tuple[int, int]) -> str:
    """
    Generate exam scheduling test input

    Args:
        N: Number of classes
        M: Number of rooms
        K: Number of conflict pairs
        d_range: Min and max number of students per class

    Returns:
        str: The formatted test input as a string
    """
    # Generate student counts for each class
    d = [random.randint(*d_range) for _ in range(N)]

    # Generate room capacities
    c = [random.randint(max(d) // 2, max(d) * 3 // 2) for _ in range(M)]
    if max(c) < max(d):
        # Ensure at least one room can accommodate the largest class
        c[0] += 10 + max(d) - c[0]

    # Create all possible conflict pairs (i < j)
    all_pairs = [(i, j) for i in range(1, N+1) for j in range(i+1, N+1)]
    if K > len(all_pairs):
        print("Error: K exceeds the maximum number of pairs")
        sys.exit(1)

    # Randomly sample K unique conflict pairs
    conflict_pairs = random.sample(all_pairs, K)
    conflict_pairs.sort()

    # Build output lines
    lines = []
    lines.append(f"{N} {M}")
    lines.append(" ".join(map(str, d)))
    lines.append(" ".join(map(str, c)))
    lines.append(str(K))
    for u, v in conflict_pairs:
        lines.append(f"{u} {v}")

    return "\n".join(lines)

if __name__ == "__main__":
    # Generate input with N=5000, M=30, K=3934002, student count in range [20, 70]
    output_text = generate(5000, 30, 3934002, (20, 70))

    # Write the generated input to a file
    with open("output.txt", 'w') as f:
        f.write(output_text)
    print("Generated input saved to output.txt")
