# Benchmarking BLIS `dgemm`: Peak FLOPS, Real-World Efficiency, and the Bottlenecks in Between

## 1. Objective

The goal of this project was to understand the BLIS (BLAS-like Library Instantiation Software) implementation of double-precision general matrix multiplication (`dgemm`), benchmark it across a wide range of matrix sizes, compare the measured throughput against the theoretical peak performance of the test machine, and explain the gap between the two.

---

## 2. Understanding the `dgemm` Interface

BLIS implements the classic BLAS Level-3 GEMM operation. The mathematical operation performed is:

```
C = alpha * (A * B) + beta * C
```

This single equation is the reason GEMM is considered the "universal" building block of dense linear algebra — with the right choice of `alpha`, `beta`, and matrix shapes, it can express plain multiplication, accumulation, rank-k updates, and more.

### Parameter breakdown

| Parameter | Meaning |
|---|---|
| `transa`, `transb` | Whether A and/or B should be used as-is, transposed, or (in complex variants) conjugate-transposed before multiplication |
| `m` | Number of rows of A (and of C) |
| `n` | Number of columns of B (and of C) |
| `k` | Inner/shared dimension — number of columns of A and rows of B |
| `alpha` | Scalar multiplier applied to the product `A*B` |
| `A` | Pointer to matrix A, dimensions `m x k` |
| `lda` | Leading dimension of A — the stride between consecutive rows/columns in memory, which lets you operate on a sub-matrix of a larger allocated array |
| `B` | Pointer to matrix B, dimensions `k x n` |
| `ldb` | Leading dimension of B |
| `beta` | Scalar multiplier applied to the existing values of C before the product is added |
| `C` | Pointer to output/accumulator matrix, dimensions `m x n` |
| `ldc` | Leading dimension of C |

Setting `alpha = 1` and `beta = 0` reduces the operation to plain matrix multiplication `C = A*B`, which is the configuration used for the benchmark runs described below.

### Why GEMM is structured this way internally

BLIS doesn't implement GEMM as three naive nested loops. It implements it as **six nested loops**, five of which exist purely to move data through the cache hierarchy in cache-sized blocks, with the sixth (innermost) being a highly hand-tuned micro-kernel written in assembly/intrinsics for the target CPU architecture. The blocking parameters are usually named `MC`, `KC`, `NC` (block sizes for the three loops that tile the problem into L3/L2/L1-sized chunks) and `MR`, `NR` (the micro-kernel's register-resident tile size). This blocking structure is precisely why memory access patterns end up being one of the two dominant bottlenecks observed in this project (see Section 5).

---

## 3. Benchmark Methodology

- **Matrix sizes tested:** square matrices ranging from 2,000 × 2,000 up to 20,000 × 20,000 (2k → 20k), scaled in steps.
- **Metric measured:** achieved GFLOPS (billion floating-point operations per second) for each matrix size.
- **FLOP count formula used:** for an `m x k` by `k x n` multiplication, the number of floating point operations is:

```
FLOPs = 2 * m * n * k
```

(one multiply + one add per output element per k-step). Dividing this by the measured wall-clock time for the multiplication gives achieved GFLOPS.

---

## 4. Calculating Theoretical Peak Performance

Theoretical peak GFLOPS was calculated using:

```
Peak GFLOPS = (Number of cores) x (Clock cycles per second) x (FLOPs per cycle per core)
```

Where:
- **Number of cores** — physical cores available to the workload (this is where the performance/efficiency core split later becomes relevant — see Section 5).
- **Clock cycles per second (clock frequency)** — typically the sustained all-core turbo frequency, not the single-core max boost, since GEMM is a multi-threaded workload.
- **FLOPs per cycle per core** — determined by:
  - SIMD width (e.g., AVX2 = 256-bit = 4 doubles per vector register, AVX-512 = 512-bit = 8 doubles per register)
  - Number of FMA (fused multiply-add) units per core (each FMA counts as 2 FLOPs)
  - So a core with 2x AVX2 FMA units achieves `2 (FMA units) x 4 (doubles/vector) x 2 (FLOPs/FMA) = 16 FLOPs/cycle`

This peak number is a hard ceiling — a theoretical maximum assuming perfect instruction throughput, zero memory stalls, zero thermal throttling, and every core running identical, fully vectorized work simultaneously. In practice, no real workload sustains this number, which is exactly what the benchmark set out to quantify.

---

## 5. Observed Results and Bottleneck Analysis

The measured GFLOPS across the 2k–20k range topped out at roughly **50% of the calculated theoretical peak**. Several compounding factors were identified as the cause.

### 5.1 Heterogeneous core architecture (Performance vs. Efficiency cores)

On modern hybrid CPUs (Intel's P-core/E-core designs, or similar big.LITTLE-style arrangements), not all cores are created equal:

- **Performance (P) cores** typically support wider SIMD units and higher per-cycle FLOP throughput.
- **Efficiency (E) cores** are narrower, lower-clocked, and often lack the same vector width or dual-FMA capability.

BLIS's multithreading model parallelizes GEMM by partitioning the problem (typically along the second loop around the micro-kernel) and assigning chunks of work to threads, usually scheduled by OpenMP or a similar threading backend, one thread per core. Because the OS thread scheduler treats P-cores and E-cores largely as interchangeable "logical cores," BLIS ends up assigning **equal-sized chunks of work to unequal-performance cores**. The result:

- P-cores finish their chunk quickly and sit idle waiting.
- E-cores lag behind, becoming the pacing item for that block of work (a classic **load-imbalance / straggler problem**).
- The effective throughput of the whole run gets dragged down toward E-core-level performance, even though the theoretical peak calculation assumed every core delivers full performance.

This mismatch between the *theoretical peak formula* (which implicitly assumes homogeneous cores) and the *actual heterogeneous hardware* is one of the two largest contributors to the ~50% ceiling.

### 5.2 Memory access bottlenecks

GEMM is FLOP-heavy, but it is also extremely sensitive to how data moves between DRAM and the cache hierarchy:

- **Cache blocking limits:** BLIS's `MC`/`KC`/`NC` block sizes are tuned for specific cache sizes. If the actual L2/L3 cache is shared unevenly across P-cores and E-cores (or is smaller than the tuning assumed), blocks that should stay cache-resident spill back to memory more often than expected.
- **Memory bandwidth saturation:** as more threads run simultaneously, they compete for the same shared memory bus. Once bandwidth is saturated, additional cores stop delivering proportional speedup — compute units sit idle waiting on data ("starved" cores).
- **NUMA/interconnect effects** (on multi-socket or multi-die systems): cores accessing memory attached to a different socket/die incur significantly higher latency.
- **Packing overhead:** BLIS "packs" sub-blocks of A and B into contiguous, cache-friendly buffers before the micro-kernel runs on them. This packing step itself consumes memory bandwidth and cycles that don't contribute directly to the useful FLOP count, silently lowering achieved GFLOPS relative to the theoretical ceiling.

### 5.3 Thermal and power-state effects

- **Thermal throttling:** sustained large matrix multiplications (10k–20k range) keep all cores at high utilization for extended periods, raising die temperature. Once thermal limits are hit, the CPU reduces clock speed to stay within its thermal envelope — directly cutting into the "clock cycles per second" term of the peak-performance formula.
- **Power/performance modes:** OS or firmware power profiles (e.g., "Balanced," "Power Saver," "Best Performance") cap boost clocks and can even reduce the number of cores allowed to boost simultaneously.
- **AC vs. battery power:** on battery, most systems enforce a lower sustained power limit (PL1/PL2-style limits) to preserve battery life, further reducing sustained clock speed compared to being plugged into AC power.

Each of these factors independently reduces the *actual* clock frequency and *actual* number of fully-boosted cores compared to the idealized values used in the theoretical peak formula — meaning the "denominator" in the efficiency ratio (measured ÷ peak) was inflated relative to what the hardware could realistically sustain run after run.

### 5.4 Summary of the ~50% efficiency figure

Efficiency (%) = Measured GFLOPS / Theoretical Peak GFLOPS

The observed ~50% ceiling is best explained as the combined effect of:

1. **Load imbalance from P-core/E-core heterogeneity** — largest single contributor, since BLIS's thread partitioning assumes uniform core performance.
2. **Memory-bound behavior at scale** — larger matrices increasingly bottleneck on bandwidth and cache-block spillage rather than raw compute.
3. **Thermal throttling** during long sustained runs, especially at the largest matrix sizes (10k–20k).
4. **Power-mode/AC-battery constraints** capping sustained clocks below the boost-clock assumption used in the peak formula.

---

## 6. Suggestions for Further Investigation

If you want to push this analysis further, some natural next steps:

- **Core affinity pinning:** use `taskset`/`numactl` (Linux) or thread affinity APIs to force BLIS threads onto P-cores only, and re-measure achieved GFLOPS to isolate the P/E-core effect from the memory-bound effect.
- **Vary thread count independently of matrix size** to find the point where memory bandwidth saturates (throughput plateaus or drops even as more cores are added).
- **Log per-core clock speed and package temperature** (e.g., via `turbostat`, `perf`, or vendor tools) during each run, so throttling events can be correlated directly with GFLOPS dips.
- **Re-tune BLIS's cache-blocking parameters** (`MC`, `KC`, `NC`) for your specific cache sizes using BLIS's `configure`/architecture-specific kernel selection, rather than relying on the generic auto-detected configuration.
- **Repeat runs under fixed power/AC conditions** to separate power-policy effects from thermal effects.

---

## 7. Conclusion

The equation `C = alpha*(A*B) + beta*C` is deceptively simple, but the journey from that equation to actual silicon throughput passes through multiple layers: BLIS's six-loop blocking strategy, the OS thread scheduler, the CPU's heterogeneous core layout, the memory hierarchy, and the power/thermal management system. This project's finding — that real-world achieved performance topped out near 50% of the naively-calculated theoretical peak — is a genuinely representative result for hybrid-core consumer CPUs running memory-bound-at-scale workloads like large dense GEMM, and it correctly identifies the two dominant causes: **P-core/E-core load imbalance** and **memory-access bottlenecks**, compounded by thermal and power-state effects at the largest matrix sizes.
