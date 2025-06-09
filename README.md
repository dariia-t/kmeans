# Parallel K-Means Clustering (Intel TBB)  

## Overview  
This project implements and benchmarks three versions of the K-Means clustering algorithm:  
- `kmeans_serial`: basic sequential version  
- `kmeans_serial_improved`: optimized sequential version  
- `kmeans_parallel`: parallel implementation using **Intel Threading Building Blocks (TBB)**  

The goal is to explore parallelism and performance optimization using multithreading and SIMD-aware design.

---

## Requirements  
- GCC 8.5.0+  
- Intel TBB  
- Linux environment (tested on Lehigh Sunlab machines)

---

## Setup & Execution  

### 1. Install Intel TBB  
Download and extract:
```bash
wget https://github.com/uxlfoundation/oneTBB/releases/download/v2022.0.0/oneapi-tbb-2022.0.0-lin.tgz
tar -xvzf oneapi-tbb-2022.0.0-lin.tgz
cd oneapi-tbb-2022.0.0/env
source vars.sh
```

### 2. Run All Versions
Use the provided script to compile and run all implementations:

```bash
chmod +x run.sh
./run.sh
```
- Outputs are logged to: kmeans_output.log
- Modify datasets in DATASETS=() inside run_all.sh as needed.

---

## Dataset Format
Input format:

```bash
<total_points> <dimensions> <K> <max_iterations> <has_name (0 or 1)>
<value_1> <value_2> ... [optional_name]
```

---

## Cleanup
Binaries are automatically removed after execution.

---

## Citations
Based on the serial implementation by [marcoscastro](https://github.com/marcoscastro/kmeans).

