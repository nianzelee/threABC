echo [INFO] Make circuit combinational
comb
echo [INFO] Perform fraig
fraig
echo [INFO] Synthesize threshold logic circuit by tech map
synth1
echo [INFO] Write out synthesized circuit as a blif file
t2b compTH_1.blif
echo [INFO] Collapse the synthesized circuit
mt
echo [INFO] Write out collapsed circuit as a blif file
t2b compTH_2.blif
echo [INFO] Perform cec over two blif files
time
cec -n compTH_1.blif compTH_2.blif
time
quit
