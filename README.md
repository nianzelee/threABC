# threABC
## Introduction
- Threshold logic collapse operation and verification within ABC
- Primary developer: Nian-Ze Lee from National Taiwan University
- C implementation of algorithms proposed in [Analytic Approaches to the Collapse Operation and Equivalence Verification of Threshold Logic Circuits](https://ieeexplore.ieee.org/document/7827582/)
- Algorithms and codes for threshold logic technology mapping proposed in [Threshold Logic Synthesis Based on Cut Pruning](https://ieeexplore.ieee.org/document/7372610/) by Augusto Neutzling et al.
- Reference: Nian-Ze Lee, Hao-Yuan Kuo, Yi-Hsiang Lai, Jie-Hong R. Jiang:
Analytic approaches to the collapse operation and equivalence verification of threshold logic circuits. ICCAD 2016: 5
## Contents
1. [Installation](#installation)
2. [Commands](#commands)
3. [Examples](#examples)
4. [Experiments](#experiments)
5. [Contact](#contact)
## Installation
Type `make` to complie and the executable file is `bin/abc`
```
make
```
The source code has been compiled successfully with GCC\_VERSION=8.2.0 under CentOS 7.3.1611/Ubuntu 18.04 LTS
## Commands:
### I/O:
- `read_th` (alias `rt`): read a TL circuit (TLC) file in the `.th` format (POs must be buffered)
- `write_th` (alias `wt`): write the current TLC out in the `.th` format
- `print_th` (alias `pt`): print the network statistics of the current TLC
### Synthesis:
- `aig2th` (alias `a2t`): convert an AIG circuit to a TLC by replacing AIG nodes with TL gates (TLGs)
- `merge_th` (alias `mt`): the proposed collapsing-based TLC synthesis
- `th2blif` (alias `t2b`): convert a TLC to a blif file by transforming a threshold gate to a truth table
### Verification:
- `th2mux` (alias `t2m`): convert a TLC to an AIG circuit by expanding a TLG to a MUX tree
- `thverify` (alias `tvr`): write a CNF/PB file for the equivalence checking of two TLCs
- `thpg` (alias `tp`): write a PB file for the output satisfiability of a TLC with PG encoding
### Misc:
- `test_th`: testing playground
- `profile_th`: print detailed collapse information
### Outdated (only for documentation):
- PB_th: eq check between pNtkCur and current_TList by PB
- CNF_th: eq check between pNtkCur and current_TList by CNF
- NZ: eq check between cut_TList and current_TList by PB
- OAO: eq check between cut_TList and current_TList by CNF
## Examples
1. Collapse an AIG circuit iteratively with a fanout bound = 100 (`aig_syn` is defined in file abc.rc)
```
abc 01> r exp_TCAD/collapse/benchmark/iscas_itc/s38417.blif
abc 02> aig_syn
abc 03> mt -B 100
abc 04> pt
abc 05> q
```
2. Collapse a synthesized TLC iteratively with a fanout bound = 100 (`tl_syn` is defined in file abc.rc)
```
abc 01> r exp_TCAD/collapse/benchmark/iscas_itc/s38417.blif
abc 02> tl_syn
abc 03> wt s38417_before_clp.th
abc 04> mt -B 100
abc 05> pt
abc 06> wt s38417_after_clp.th
abc 07> q
```
3. Verify equivalence using the TL-to-MUX translation and ABC command `cec` (continued from the above example)
```
abc 01> rt s38417_before_clp.th
abc 02> t2m
abc 03> w s38417_before_clp.aig
abc 04> rt s38417_after_clp.th
abc 05> t2m
abc 06> w s38417_after_clp.aig
abc 07> cec s38417_before_clp.aig s38417_after_clp.aig
abc 08> q
```
4. Verify equivalence using the TL-to-PB translation and `minisat+` (continued from the above example)
```
abc 01> tvr s38417_before_clp.th s38417_after_clp.th
abc 02> q
$ bin/minisat+ compTH.opb
```
5. Verify equivalence using the TL-to-CNF translation and `minisat` (continued from the above example)
```
abc 01> tvr -V 1 s38417_before_clp.th s38417_after_clp.th
abc 02> q
$ bin/minisat compTH.dimacs
```
6. Check the output satisfiability of a TLC using the PB-based method with PG encoding (`pg_and` is defined in file abc.rc)
```
abc 01> r exp_TCAD/pg_encoding/benchmark/b14.blif
abc 02> pg_and
abc 03> q
$ bin/minisat+ no_pg.opb
$ bin/minisat+ pg.opb
```
## Experiments
Directory `exp_TCAD` contains all benchmarks, scripts, log files, and data in the experiments. Specifically:
1. Sub-directory `collapse` is for the collapsing-based synthesis experiments
2. Sub-directory `eqcheck` is for the verification experiments between TLCs before and after collapsing
3. Sub-directory `pg_encoding` is for the PG encoding experiments
4. Sub-directory `high_fanin` is for the translation scalability experiments
5. Sub-directory `dnn_mnist` is for the activation-binarized neural networks experiments
## Suggestions, Questions, Bugs, etc
You are welcome to [create an issue](https://github.com/nianzelee/ssatABC/issues) to make suggestions, ask questions, or report bugs, etc.
## Contact
Please send an email to Nian-Ze Lee (nianzelee@gmail.com) if there is any problem.
