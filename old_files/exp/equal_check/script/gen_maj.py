'''*************************************************************
FileName    [gen_maj.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script to generate majority functions.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [9, August, 2018]
*******************************************************************'''

import subprocess as sp

n_list = [3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 63, 127, 255, 511, 1023, 2047]
t_list = [1]
#t_list = [0, 1, 2]

for n in n_list:
   for t in t_list:
      cmd = 'python ../script/maj.py -t %d -n %d' % (t, n)
      sp.run(cmd, shell=True)
