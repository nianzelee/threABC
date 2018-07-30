### threABC
Threshold logic operation within ABC
Developed by Nian-Ze Lee and Hao-Yuan Kuo from NTU

### Commands:
# I/O:
read_th
write_th
print_th: print network statistics of current_TList
# Synthesis:
aig2th: convert AIG to TH by replacing an AND gate as a threshold gate [1,1;2]
merge_th: reduce # of threshold gates by collapsing
th2mux: convert TH to AIG by expanding a threshold gate to a mux tree
# Verification:
EC_th
CNF_th
th2blif
# misc:
test_th: testing playground
profile_th: print detailed collapse information
