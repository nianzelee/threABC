'''*************************************************************
FileName    [pb.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for random TL functions with pb.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [12, June, 2019]
*******************************************************************'''

import os
import time
import numpy as np
import pandas as pd
import subprocess as sp

a = [16, 20, 24, 28, 32, 36, 40]
N = 100
time_df = pd.DataFrame(np.zeros(shape=(N, len(a))), columns=a)
size_df = pd.DataFrame(np.zeros(shape=(N, len(a))), columns=a)
for n_input in a:
   for i in range(N):
      v = 'exp_TCAD/high_fanin/rand_tl/%d_%d.th' % (n_input, i)
      cmd = './bin/abc -c \"thverify ' + v + ' ' + v + '\"'
      start = time.time()
      sp.run(cmd, shell=True)
      elapsed = time.time()-start
      time_df.at[i, n_input] = elapsed
      size_df.at[i, n_input] = os.path.getsize('./compTH.opb')
time_df.to_csv('exp_TCAD/high_fanin/rand_csv/time_pb.csv')
size_df.to_csv('exp_TCAD/high_fanin/rand_csv/size_pb.csv')
