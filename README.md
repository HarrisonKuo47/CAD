# CAD
implement CAD tools for VLSI design

📖 VLSI CAD Tools in C++
This repository contains a suite of C++ programs that model fundamental algorithms in Computer-Aided Design (CAD) for Very-Large-Scale Integration (VLSI) circuits. Developed as part of the "CAD for VLSI Design" course at National Central University, these projects implement key stages of the design flow from scratch, including benchmark translation, circuit partitioning, static timing analysis, and analog floorplanning.

The primary focus is on a deep understanding of the core algorithms, implemented with a focus on efficiency and accuracy.


🛠️ Implemented Tools
This repository is a collection of four major programming assignments (PAs), each tackling a different aspect of VLSI design automation.


#1. Benchmark Translator (Netlist to Verilog)
This tool serves as a front-end utility to parse a standard circuit representation and convert it into a hardware description language.

Functionality: Translates ISCAS'85-style .netlist files into structural, gate-level Verilog (.v) files.

Parsing: Correctly identifies primary inputs, primary outputs, wires, and gate instances (e.g., and, or, not, nand).

Translation: Generates a complete Verilog module with correct port declarations and gate instantiations, accurately mapping net connections to gate ports.

Verification: The output Verilog is designed to be synthesizable and verifiable against a testbench using standard EDA tools like ncverilog.

#2. Circuit Partitioning (Fiduccia-Mattheyses)
This project implements the classic Fiduccia-Mattheyses (FM) heuristic to solve the netlist partitioning problem, a critical first step in physical design.

Algorithm: Performs two-way partitioning on a gate-level netlist with the primary objective of minimizing the cut size (the number of nets that span both partitions).

Initialization: Employs a Greedy algorithm based on connectivity scores to create a reasonable initial partition.

Optimization: Iteratively refines the partition by moving the single cell that provides the highest gain (reduction in cut size).

Features: Implements core FM concepts including cell locking after a move, dynamic gain updates for neighboring cells, and a rollback mechanism to restore the partition state that yielded the minimum cut size during a pass.

#3. Graph-Based Static Timing Analysis (STA)
-A critical tool for verifying that a circuit meets its performance targets without running exhaustive simulations. This STA engine calculates signal propagation delays through a combinational circuit.

-Parsing: Reads a gate-level Verilog netlist and a standard cell timing library in Liberty (.lib) format.

-Circuit Modeling: Constructs a Directed Acyclic Graph (DAG) to represent the circuit's structure and timing dependencies.

-Timing Calculation:
   Calculates the capacitive load on each net based on fanout.
   Determines cell propagation delays and output transition times by looking up values in Non-Linear Delay Models (NLDM) from the Liberty file.
   Implements bi-linear interpolation and extrapolation to accurately find timing values from the NLDM tables.

Analysis: Traverses the graph in topological order to propagate arrival times and ultimately identifies the longest (critical) and shortest paths in the circuit.

4. Analog Floorplanner (Simulated Annealing)
-This tool addresses the complex challenge of analog layout, where both physical compactness and electrical performance are paramount.

-Problem: Solves a rectangle packing problem for analog modules, aiming to produce a legal, non-overlapping floorplan.

-Optimization Algorithm: Utilizes Simulated Annealing (SA), a powerful probabilistic metaheuristic, to explore the vast solution space.

-Cost Function: 
   Optimizes a sophisticated, multi-objective cost function that includes:
      #1. Bounding Box Area and Aspect Ratio
      #2. Integral Nonlinearity (INL), a key metric for analog performance and layout uniformity.
      #3. A massive penalty for any module overlap to ensure a legal layout.

Perturbation Strategy: The SA search is driven by a mix of random moves: moving a module, swapping the locations of two modules, and changing a module's shape/variant.

📂 Proposed Directory Structure
To organize these projects, the following structure is recommended:

vlsi-cad-tools/
├── benchmark_translator/         # Netlist to Verilog translator
├── circuit_partition/        # FM algorithm implementation
├── graph_based_STA/                # Static Timing Analysis tool
├── analog_floorplan/       # Analog block floorplanning 
├── Makefile                # Unified build script
└── README.md               # Project overview (this file)

⚙️ Build & Run
Requirements
C++11 or later
GNU Make / g++
Build All Tools
Bash



Author
Tsu-Hao Kuo (郭子浩)

Graduate Student @ National Central University, ICS Lab

Focus: Digital CAD, Timing Analysis, Physical Design
