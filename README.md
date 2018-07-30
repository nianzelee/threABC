# threABC
## Introduction
- Threshold logic operation within ABC
- Developed by Nian-Ze Lee (NZ) and Hao-Yuan Kuo (OAO) from National Taiwan University
- C implementation of algorithms proposed in [Analytic Approaches to the Collapse Operation and Equivalence Verification of Threshold Logic Circuits](https://ieeexplore.ieee.org/document/7827582/)
- Algorithms and codes for threshold logic technology mapping proposed in [Threshold Logic Synthesis Based on Cut Pruning](https://ieeexplore.ieee.org/document/7372610/) by Augusto Neutzling, et al.
## Contents
1. [Installation](#installation)
2. [Commands](#commands)
3. [Examples](#examples)
4. [Contact](#contact)
## Installation
Type `make` to complie and the executable is `bin/abc`
```
make
```
It has been tested successfully under CentOS 7.3.1611 with GCC\_VERSION=4.8.5
## Commands:
### I/O:
- read_th: read a .th file (PO must be buffered)
- write_th: write current_TList out as a .th file
- print_th: print network statistics of current_TList
### Synthesis:
- aig2th:   convert AIG to TH by replacing an AND gate as a threshold gate [1,1;2]
- merge_th: reduce # of threshold gates by collapsing
- th2blif:  convert TH to a blif file by transforming a threshold gate to the truth table
- th2mux:   convert TH to AIG by expanding a threshold gate to a mux tree
### Verification:
- PB_th: eq check between pNtkCur and current_TList by PB
- CNF_th: eq check between pNtkCur and current_TList by CNF
- NZ: eq check between cut_TList and current_TList by PB
- OAO: eq check between cut_TList and current_TList by CNF
### misc:
- test_th: testing playground
- profile_th: print detailed collapse information
## Examples
1. Collapse a synthesized threshold logic circuit
```
abc 01> r benchmarks/iscas85/c6288.bench
abc 02> synth1
abc 03> merge_th
```
2. Verify equivalence using mux-based conversion and cec
(Continued from the above example)
```
abc 04> t2m
abc 05> cec -n benchmarks/iscas85/c6288.bench
```
3. Verify equivalence using PB-based conversion and minisat+
(Continued from the above example)
```
abc 06> NZ
abc 07> quit
bin/minisat+ compTH.opb
```
You can observe that PB-based equivalence checking takes much longer time.
## Contact:
Please let us know if you have any problem using the code.  
Nian-Ze Lee: d04943019@ntu.edu.tw, nianzelee@gmail.com
