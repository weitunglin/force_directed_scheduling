# Force-Directed Latency Constraint Scheduling
Force-Directed Latency Constraint Scheduling for BLIF format written in c++

# Requirements
* cmake3.14+
* gcc7+

# Build Force-Directed Scheduling
```sh
git clone https://github.com/weitunglin/force_directed_scheduling.git
cd force_directed_scheduling
cmake -B build . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

# Run Force-Directed Scheduling

### Synopsis
```sh
<PATH_TO_LCS> <PATH_TO_BLIF> <LATENCY_CONSTRAINT>
```
### Example
```sh
./build/lcs aoi_benchmark/aoi_sample02.blif 5
```

# Worst case
```sh
./build/lcs aoi_benchmark/aoi_worstcase.blif 10
```
