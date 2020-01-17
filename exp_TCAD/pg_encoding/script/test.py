'''*************************************************************
FileName    [exp_pg.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for PG encoding experiment.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [17, January, 2020]
*******************************************************************'''

import os
import argparse
import numpy as np
import pandas as pd
import subprocess as sp

bch = "b14"
suf = "orpos"
opb = "exp_TCAD/pg_encoding/opb/" + bch + "_no_pg_" + suf + ".opb"
log = "exp_TCAD/pg_encoding/log/" + bch + "_no_pg_" + suf + ".log"
cmd = './bin/minisat+ ' + opb + " &> " + log
print(cmd)
sp.run(cmd, shell=True)
