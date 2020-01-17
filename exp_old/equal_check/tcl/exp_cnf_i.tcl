echo [INFO] Make circuit combinational
comb
echo [INFO] Perform fraig
fraig
echo [INFO] Synthesize threshold logic circuit by tech map
synth1
echo [INFO] Iteratively collapse the synthesized circuit
mt -B 100
echo [INFO] Write out CNF equivalence checking file
OAO
quit
