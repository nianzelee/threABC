echo [INFO] Make circuit combinational
comb
echo [INFO] Perform fraig
fraig
echo [INFO] Synthesize threshold logic circuit by tech map
synth1
echo [INFO] Write out synthesized circuit as an AIG file
t2m
w compTH_1.aig
echo [INFO] Collapse the synthesized circuit
mt
echo [INFO] Write out collapsed circuit as an AIG file
t2m
w compTH_2.aig
echo [INFO] Perform cec over two AIG files
time
cec -n compTH_1.aig compTH_2.aig
time
quit
