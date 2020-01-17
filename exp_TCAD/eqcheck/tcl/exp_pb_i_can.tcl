echo [INFO] Make circuit combinational
comb
echo [INFO] Perform fraig
fraig
echo [INFO] Synthesize threshold logic circuit by tech map
synth1
echo [INFO] Write out the first TH circuit
wt compTH_1.th
echo [INFO] Collapse the synthesized circuit
mt -B 100
echo [INFO] Write out the second TH circuit
wt compTH_2.th
quit
