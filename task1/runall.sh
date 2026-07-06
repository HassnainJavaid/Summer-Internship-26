#!/bin/bash

# Let BLIS auto-factor threads across its JC/IC/JR loops rather than
# forcing all parallelism into one loop (that causes redundant packing
# and kills scaling). Only set the total thread count.
export BLIS_NUM_THREADS=$(nproc)
export OMP_NUM_THREADS=$(nproc)

# Pin OpenMP threads to cores and keep them close to avoid migration
# destroying cache locality mid-run.
export OMP_PROC_BIND=close
export OMP_PLACES=cores

echo "Compiling code natively mapped for your i7 architecture..."
gcc main.c -O3 -march=native -mavx2 -mfma -funroll-loops -fopenmp \
    -I/usr/local/include -L/usr/local/lib -lblis -lm -lpthread -o benchmark

if [ ! -f ./benchmark ]; then
    echo "ERROR: compilation failed, 'benchmark' binary was not produced."
    exit 1
fi

# Wipe out past records clean
rm -f blis_benchmark_results.csv

# Initialize Master Column Headers
echo "Run,MatrixSize,Threads,TimeSeconds,FLOPs,GFLOPS" > blis_benchmark_results.csv

# Arrays scaling out all spreadsheet data points up to 20k
sizes=(2000 4000 6000 8000 10000 12000 14000 16000 18000 20000)

for N in "${sizes[@]}"
do
    echo "------------------------------------------------"
    echo "Running dimension: ${N}x${N} on All Threads"
    echo "------------------------------------------------"

    for run in {1..10}
    do
        echo " -> Loop Calculation Trial #$run"
        ./benchmark $N $run
    done
done

echo ""
echo "Execution processing finished safely."
echo "Your tracking records are ready in: blis_benchmark_results.csv"