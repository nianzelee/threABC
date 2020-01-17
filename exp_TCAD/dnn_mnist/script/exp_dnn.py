'''*************************************************************
FileName    [exp_dnn.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for DNN experiment.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [21, June, 2019]
*******************************************************************'''

import os
import subprocess as sp
a = [16, 32, 48, 64, 128]
b = [3, 4, 5]
for n_input in a:
   for n_layer in b:
      N = 'exp_TCAD/dnn_mnist/benchmark/binarized_dnn_%d_%d_256.th' % (n_input, n_layer)
      M = 'exp_TCAD/dnn_mnist/benchmark/binarized_dnn_%d_%d_16.th' % (n_input, n_layer)
      f = 'exp_TCAD/dnn_mnist/opb/%d_%d.opb' % (n_input, n_layer)
      g = 'exp_TCAD/dnn_mnist/log/%d_%d.log' % (n_input, n_layer)
      cmd = './bin/abc -c \"thverify ' + N + ' ' + M + '\"'
      sp.run(cmd, shell=True)
      cmd = 'mv compTH.opb ' + f
      sp.run(cmd, shell=True)
      cmd = './bin/minisat+ ' + f + ' > ' + g + ' &'
      sp.run(cmd, shell=True)
