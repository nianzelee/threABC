# threABC
- Threshold logic operation within ABC
- Developed by Nian-Ze Lee (NZ) and Hao-Yuan Kuo (OAO) from National Taiwan University
- C implementation of algorithms proposed in [Analytic Approaches to the Collapse Operation and Equivalence Verification of Threshold Logic Circuits](https://ieeexplore.ieee.org/document/7827582/)

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

## Contact:
Please let us know if you have any problem using the code.  
Nian-Ze Lee: d04943019@ntu.edu.tw, nianzelee@gmail.com
