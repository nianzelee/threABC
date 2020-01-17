'''*************************************************************
FileName    [exp.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for random NN experiment.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [11, June, 2019]
*******************************************************************'''

import os
import subprocess as sp
a = [32, 48, 64, 128]
b = [3, 4, 5]
for n_input in a:
   for n_layer in b:
      N = 'exp/high_fanin/rand_nn/%d_%d.th' % (n_input, n_layer)
      M = 'exp/high_fanin/rand_nn/%d_%d_q.th' % (n_input, n_layer)
      f = 'exp/high_fanin/rand_opb/%d_%d.opb' % (n_input, n_layer)
      g = 'exp/high_fanin/rand_log/%d_%d.log' % (n_input, n_layer)
      cmd = './bin/abc -c \"thverify ' + N + ' ' + M + '\"'
      sp.run(cmd, shell=True)
      cmd = 'mv compTH.opb ' + f
      sp.run(cmd, shell=True)
      cmd = 'timeout 7200 ./bin/minisat+ ' + f + ' &> ' + g + ' &'
      sp.run(cmd, shell=True)
