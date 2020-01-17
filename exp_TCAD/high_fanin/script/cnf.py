'''*************************************************************
FileName    [cnf.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for random TL functions with cnf.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [12, June, 2019]
*******************************************************************'''

import os
import time
import numpy as np
import pandas as pd
import subprocess as sp

a  = [16, 20, 24, 28, 32, 36, 40]
N  = 100
TO = 7200
MO = 1000000000
time_df = pd.DataFrame(np.zeros(shape=(N, len(a))), columns=a)
size_df = pd.DataFrame(np.zeros(shape=(N, len(a))), columns=a)
for n_input in a:
   for i in range(N):
      v = 'exp_TCAD/high_fanin/rand_tl/%d_%d.th' % (n_input, i)
      cmd = './bin/abc -c \"thverify -V 1 ' + v + ' ' + v + '\"'
      try:
         start = time.time()
         sp.call(cmd, shell=True, timeout=TO)
         elapsed = time.time()-start
         time_df.at[i, n_input] = elapsed
         size_df.at[i, n_input] = os.path.getsize('./compTH.dimacs')
      except sp.TimeoutExpired:
         time_df.at[i, n_input] = TO
         size_df.at[i, n_input] = MO
time_df.to_csv('exp_TCAD/high_fanin/rand_csv/time_cnf.csv')
size_df.to_csv('exp_TCAD/high_fanin/rand_csv/size_cnf.csv')
