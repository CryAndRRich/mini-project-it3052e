# test-case:
Input size (N, M, K)
- 15 tests Type 1: 
(5, 2, 4)   (6, 2, 7)   (7, 3, 10)   (8, 3, 10)   (9, 4, 12)
(10, 3, 16) (11, 4, 21) (12, 4, 32)  (13, 4, 41)  (14, 4, 70)
(15, 4, 92) (16, 4, 94) (17, 4, 113) (18, 5, 115) (19, 5, 131)

- 10 tests Type 2:
(50, 10, 491) (51, 10, 442) (52, 11, 503) (53, 11, 600) (54, 11, 594)
(55, 11, 633) (56, 12, 617) (57, 12, 662) (58, 12, 649) (59, 13, 700)

- 10 tests Type 3:
(100, 10, 2430)  (200, 10, 8720)   (300, 10, 22415)  (400, 10, 40120)  (500, 10, 61210)
(600, 11, 89210) (700, 11, 122340) (800, 11, 154000) (900, 10, 205130) (1000, 10, 247228)

- 03 test Type 4:
(1000, 20, 247448)  (2000, 20, 823453) (3000, 20, 1010240)

# mini-project-it3052e
```
optimization-algorithms/
├── 01_constructive_local_search/
│   ├── greedy/                        # GA: Greedy construction
│   │   ├── greedy_01.c
│   │   ├── greedy_02.c
│   │   └── greedy_03.c
│   ├── local_search/                  
│   │   ├── ls_01.c                    # LS: Simple neighborhood search
│   │   ├── ls_02.c
│   │   ├── ls_03.c
│   │   └── ils.c                      # ILS (Iterated Local Search): Perturb and Restart
│   ├── simulated_annealing/           # SA: Probabilistic hill-climbing
│   │   ├── sa_01.c
│   │   └── sa_02.c
│   ├── tabu_search/                   # TS: Memory-based local search
│   │   └── ts.c
│
├── 02_evolutionary_population_based/
│   ├── genetic_algorithm/             # GA: selection+crossover+mutation
│   │   └── genetic_algorithm.c
│   ├── differential_evolution/        # DE: mutation by vector differences
│   │   └── differential_evolution.c
│   ├── cma_es/                        # CMA-ES: covariance adaptation ES
│   │   └── cma_es.c
│   ├── estimation_of_distribution/    # EDA: learn & sample probability models
│   │   └── eda_umda.c
│   └── memetic_algorithm/             # Hybrid GA + local search
│       └── memetic_algorithm.c
│
├── 03_swarm_intelligence/
│   ├── particle_swarm_optimization/   # PSO: velocity-position updates
│   │   └── pso.c
│   ├── ant_colony_optimization/       # ACO: pheromone-guided tours
│   │   └── aco.c
│   ├── artificial_bee_colony/         # ABC: employed/onlooker/scout bees
│   │   └── abc.c
│   ├── cuckoo_search/                 # CS: Lévy flights + brood parasitism
│   │   └── cuckoo_search.c
│   ├── firefly_algorithm/             # FA: attractiveness-based moves
│   │   └── firefly.c
│   └── bat_algorithm/                 # Bat: echolocation-inspired search
│       └── bat.c
│
├── 04_probabilistic_and_tree_search/
│   ├── cross_entroc_method/           # CE: iterative distribution refitting
│   │   └── cross_entroc.c
│   └── monte_carlo_tree_search/       # MCTS: tree + simulation
│       └── mcts.c
│
└── 05_specialized_and_hybrid/
    ├── harmony_search/                # HS: music-inspired pitch adjustment
    │   └── hs.c
    ├── bacterial_foraging/            # BFO: chemotaxis + reproduction
    │   └── bacterial_foraging.c
    ├── clonal_selection_algorithm/    # CSA: immune-inspired algorithms
    │   └── ais.c
    └── beam_search/                   # BS: k-best breadth-first search
        ├── bs_01.c
        └── bs_02.c
```
