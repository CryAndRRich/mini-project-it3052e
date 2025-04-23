# mini-project-it3052e
```
optimization-algorithms/
├── 01_constructive_local_search/
│   ├── greedy/                        # Greedy construction
│   │   └── greedy.py
│   ├── local_search/                  # Simple neighborhood search
│   │   └── local_search.py
│   ├── simulated_annealing/           # SA: probabilistic hill-climbing
│   │   └── simulated_annealing.py
│   ├── tabu_search/                   # Memory-based local search
│   │   └── tabu_search.py
│   ├── variable_neighborhood_search/  # VNS: dynamic neighborhoods
│   │   └── vns.py
│   ├── iterated_local_search/         # ILS: perturb-and-restart
│   │   └── iterated_local_search.py
│   └── grasp/                         # GRASP: randomized greedy + LS
│       └── grasp.py
│
├── 02_evolutionary_population_based/
│   ├── genetic_algorithm/             # GA: selection+crossover+mutation
│   │   └── genetic_algorithm.py
│   ├── differential_evolution/        # DE: mutation by vector differences
│   │   └── differential_evolution.py
│   ├── cma_es/                        # CMA-ES: covariance adaptation ES
│   │   └── cma_es.py
│   ├── estimation_of_distribution/    # EDA: learn & sample probability models
│   │   └── eda_umda.py
│   └── memetic_algorithm/             # Hybrid GA + local search
│       └── memetic_algorithm.py
│
├── 03_swarm_intelligence/
│   ├── particle_swarm_optimization/   # PSO: velocity-position updates
│   │   └── pso.py
│   ├── ant_colony_optimization/       # ACO: pheromone-guided tours
│   │   └── ant_colony.py
│   ├── artificial_bee_colony/         # ABC: employed/onlooker/scout bees
│   │   └── abc.py
│   ├── cuckoo_search/                 # CS: Lévy flights + brood parasitism
│   │   └── cuckoo_search.py
│   ├── firefly_algorithm/             # FA: attractiveness-based moves
│   │   └── firefly.py
│   └── bat_algorithm/                 # Bat: echolocation-inspired search
│       └── bat.py
│
├── 04_probabilistic_and_tree_search/
│   ├── cross_entropy_method/          # CE: iterative distribution refitting
│   │   └── cross_entropy.py
│   └── monte_carlo_tree_search/       # MCTS: tree + simulation
│       └── mcts.py
│
└── 05_specialized_and_hybrid/
    ├── harmony_search/                # HS: music-inspired pitch adjustment
    │   └── harmony_search.py
    ├── bacterial_foraging/            # BFO: chemotaxis + reproduction
    │   └── bacterial_foraging.py
    ├── artificial_immune_system/       # AIS: immune-inspired algorithms
    │   └── ais.py
    └── beam_search/                   # Beam: k-best breadth-first search
        └── beam_search.py
```
