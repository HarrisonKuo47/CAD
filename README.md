# VLSI CAD Tools (C++)

A compact suite of C++ programs implementing fundamental algorithms in VLSI CAD, developed as part of the “CAD for VLSI Design” course at National Central University (NCU). The repository focuses on correctness, algorithmic clarity, and practical usability across core stages of the design flow:

- Benchmark translation (ISCAS’85 netlist → structural Verilog)
- Circuit partitioning (Fiduccia–Mattheyses heuristic)
- Graph-based Static Timing Analysis (STA) with Liberty NLDM
- Analog floorplanning (Simulated Annealing)

Author: Tsu-Hao Kuo (郭子浩)  
Affiliation: Graduate Student @ NCU  
Focus: Digital CAD, Timing Analysis, Physical Design

---

## Contents

- Overview
- Implemented Tools
- Repository Structure
- Build & Run
- Example Usage
- Input Formats & Data
- Results & Reports
- Roadmap
- Contributing
- License
- Acknowledgements

---

## Overview

This repository provides clean reference implementations of essential VLSI CAD tasks, written from scratch in modern C++. Each subproject is self-contained yet consistent in style, flags, and output conventions. The goals are:
- Reinforce understanding of core CAD algorithms
- Offer reproducible baselines and clear I/O interfaces
- Enable experimentation, benchmarking, and extension

Target audience includes students, researchers, and engineers seeking educational yet practical CAD tools.

---

## Implemented Tools

1) Benchmark Translator (ISCAS’85 netlist → Verilog)
- Parses ISCAS’85-style gate-level .netlist
- Generates synthesizable, structural Verilog (.v)
- Correctly identifies PIs/POs, wires, and gate instances (and, or, not, nand, …)
- Output suitable for EDA tools (e.g., ncverilog) and downstream analysis

2) Circuit Partitioning (Fiduccia–Mattheyses)
- Two-way min-cut partitioning on gate-level netlists
- Greedy initialization + FM passes with highest-gain moves
- Cell locking, dynamic gain updates, and rollback to best cut per pass
- Balance constraint and iteration/seed controls

3) Graph-Based Static Timing Analysis (STA)
- Builds a DAG from gate-level Verilog + Liberty (.lib)
- Computes loads, cell delays, transitions using NLDM tables
- Bi-linear interpolation/extrapolation for accurate table lookup
- Topological propagation to report arrival times, critical/shortest paths

4) Analog Floorplanner (Simulated Annealing)
- Rectangle packing for analog modules (non-overlap, compactness)
- SA with multiple perturbations: move / swap / shape-variant change
- Multi-objective cost:
  - Bounding box area and aspect ratio
  - Integral Nonlinearity (INL)
  - Large overlap penalty to enforce legality

---

## Repository Structure

```
vlsi-cad-tools/
├── benchmark_translator/        # Netlist → Verilog
│   ├── src/
│   ├── include/
│   ├── tests/
│   └── Makefile
├── circuit_partition/           # Fiduccia–Mattheyses (FM)
│   ├── src/
│   ├── include/
│   ├── tests/
│   └── Makefile
├── graph_based_STA/             # Static Timing Analysis
│   ├── src/
│   ├── include/
│   ├── tests/
│   └── Makefile
├── analog_floorplan/            # Simulated Annealing floorplanner
│   ├── src/
│   ├── include/
│   ├── tests/
│   └── Makefile
├── benchmarks/                  # Example netlists/libs/floorplan data (place here)
├── scripts/                     # Helper scripts (optional)
├── Makefile                     # Top-level build convenience (optional)
└── README.md
```

You can adopt this layout or keep your own, but the examples below assume this structure.

---

## Build & Run

Requirements
- C++11 or later
- GNU Make / g++
- (Optional) CMake ≥ 3.16
- Standard libraries for parsing text/CSV; no external deps assumed

Build (per tool)
```bash
# Example: build the translator
cd benchmark_translator
make -j

# Example: build the FM partitioner
cd circuit_partition
make -j

# Example: build the STA engine
cd graph_based_STA
make -j

# Example: build the floorplanner
cd analog_floorplan
make -j
```

Build (top-level, if provided)
```bash
# Build all tools from repo root
make -j
```

Artifacts are typically placed in `bin/` inside each tool directory (adjust per your Makefile).

---

## Example Usage

Below are canonical CLI examples. Actual flags may differ depending on your implementation; adapt as needed.

1) Benchmark Translator
```bash
# Input: ISCAS’85-style .netlist
# Output: structural Verilog (.v)
./benchmark_translator/bin/translator \
  -i benchmarks/iscas85/c17.netlist \
  -o out/c17.v
```

2) Circuit Partitioning (FM)
```bash
# Min-cut two-way partition; adjust balance and iterations
./circuit_partition/bin/partition \
  -net benchmarks/graphs/sample.net \
  -bal 0.45 \
  -max-iter 20 \
  -seed 42 \
  -rpt out/partition_report.txt
```

3) Graph-Based STA
```bash
# Verilog + Liberty + (optional) SDC constraints
./graph_based_STA/bin/sta \
  -v out/c17.v \
  -lib benchmarks/lib/Nangate45.lib \
  -sdc benchmarks/constraints/c17.sdc \
  -rpt out/sta_report.txt
```

4) Analog Floorplanner (SA)
```bash
# Blocks/nets + SA configuration; outputs floorplan and report
./analog_floorplan/bin/afp \
  -blk benchmarks/afp/blocks.txt \
  -conn benchmarks/afp/nets.txt \
  -cfg benchmarks/afp/sa.cfg \
  -out out/floorplan.svg \
  -rpt out/floorplan_report.txt
```

---

## Input Formats & Data

Benchmarks (suggested)
- Digital: ISCAS’85, MCNC, or your own netlists
- Liberty: public libs (e.g., Nangate45) for STA
- Floorplanning: simple text formats for module dimensions, shape variants, and net connectivity

Typical formats

- ISCAS’85 Netlist (simplified):
  ```
  INPUT(a) INPUT(b) OUTPUT(z)
  n1 = NAND(a, b)
  z  = INV(n1)
  ```
- Structural Verilog (output):
  ```verilog
  module c17(input a, b, output z);
    wire n1;
    nand U1 (n1, a, b);
    not  U2 (z, n1);
  endmodule
  ```
- Liberty (.lib): Standard Liberty syntax with NLDM tables
- Floorplanning blocks.txt (example):
  ```
  # name width height [variants...]
  M1  20.0  15.0
  M2  10.0  30.0
  ```
- Floorplanning nets.txt (example):
  ```
  # net: module1 module2 [module3 ...]
  N1: M1 M2
  N2: M2 M3
  ```
- SA config (sa.cfg) example:
  ```
  seed=42
  init_temp=1.0
  final_temp=1e-4
  cooling=0.90
  max_moves_per_temp=2000
  w_area=1.0
  w_aspect=0.2
  w_inl=0.5
  overlap_penalty=1e6
  ```

---

## Results & Reports

Each tool writes compact text/CSV reports and (where applicable) visualizations:
- Translator: generated `.v` files and simple summary logs
- Partitioning: cutsize per pass, best cut, partition membership
- STA: arrival/required times, slacks, critical path dump
- Floorplanner: final cost breakdown (area/aspect/INL/overlap), accepted moves, SVG layout

Place outputs under `out/` (recommended) to keep the repo tidy.

---

## Roadmap

- Add unit tests and golden references for all parsers
- Provide minimal sample benchmarks in `benchmarks/`
- Optional CMake build and CI integration
- Extend STA to support constraints (multi-clock, I/O delays) more fully
- Floorplanner: add rotated blocks and soft macro shaping

---

## Contributing

Contributions (bugs, docs, benchmarks, features) are welcome:
- Fork and create a feature branch
- Use consistent C++11 style (RAII, `unique_ptr`, `constexpr` where appropriate)
- Add comments and minimal tests for new parsers/IO
- Open a PR with a clear description and sample I/O

---

## License

Specify a license that fits your goals (e.g., MIT/BSD/GPL).  
Add a `LICENSE` file at the repo root. Until then, consider this “All rights reserved” by default.

---

## Acknowledgements

- National Central University, CAD for VLSI Design by Prof. Yu-Guang Chen
- ISCAS’85, Nangate Liberty, and other public datasets
- Classic references: FM partitioning, Liberty NLDM, Simulated Annealing

If you use this repository in academic work, please cite appropriately or reference this README.

---
