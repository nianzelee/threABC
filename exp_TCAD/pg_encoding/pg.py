'''*************************************************************
FileName    [pg.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for PG encoding experiment.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [14, June, 2019]
*******************************************************************'''

import os
import time
import numpy as np
import pandas as pd
import subprocess as sp

bench = ['b14', 'b15', 'b17', 'b18', 'b19', 'b20', 'b21', 'b22']
col   = ['c_npg', 'c_pg', 's_npg', 's_pg', 't_npg', 't_pg']
df    = pd.DataFrame(np.zeros(shape=(len(bench), len(col))), index=bench, columns=col)
for bch in bench:
   read_path = 'benchmarks/itc99/%s.blif' % bch
   cmd = './bin/abc -c \"r ' + read_path + '; oo\"'
   sp.run(cmd, shell=True)
   df.at[bch, 'c_npg'] = sum(1 for line in open('no_pg.opb'))-1
   df.at[bch, 'c_pg']  = sum(1 for line in open('pg.opb'))-1
   df.at[bch, 's_npg'] = os.path.getsize('./no_pg.opb')
   df.at[bch, 's_pg']  = os.path.getsize('./pg.opb')
   cmd = './bin/minisat+ no_pg.opb'
   start = time.time()
   sp.run(cmd, shell=True)
   df.at[bch, 't_npg'] = time.time()-start
   cmd = './bin/minisat+ pg.opb'
   start = time.time()
   sp.run(cmd, shell=True)
   df.at[bch, 't_pg'] = time.time()-start
df.to_csv('exp/pg_encoding/pg_orpos.csv')
