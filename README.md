Summer-Internship-26

A log of tasks, projects, and learnings from my Summer 2026 internship. Each task gets its own folder and a short write-up; this README is the index.

Tasks

Task 1: BLIS dgemm Benchmarking & Performance Analysis

Studied the BLIS dgemm routine, benchmarked matrix multiplication from 2k×2k to 20k×20k, computed theoretical peak CPU GFLOPS, and analyzed why achieved performance topped out at ~50% of peak (P-core/E-core load imbalance, memory bandwidth/cache-blocking limits, thermal throttling, power modes).


Folder: task-1-blis-dgemm/
Report: task-1-blis-dgemm/blis_dgemm_benchmarking_report.md


Task 2: TBD

Add a short description here once started.


Folder: task-2-.../


<!-- Add a new "### Task N" entry each time you start a new task -->
Repo structure

.
├── README.md
├── task-1-blis-dgemm/
│   ├── README.md              # Task-specific notes
│   ├── blis_dgemm_benchmarking_report.md
│   ├── src/                   # Benchmark code
│   └── results/                # Raw GFLOPS measurements / plots
├── task-2-.../
└── ...

Conventions


Each task lives in its own task-N-short-name/ folder.
Each task folder has its own short README.md explaining the goal, approach, and findings.
Larger write-ups or reports go in the task folder as Markdown files.
This top-level README is updated with a one-line summary + link every time a new task is added.


Author
