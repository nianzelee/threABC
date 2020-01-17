'''*************************************************************
FileName    [exp_blif.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for BLIF EC experiment.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [30, July, 2018]
*******************************************************************'''

import os
import argparse
import subprocess as sp
parser = argparse.ArgumentParser( description = "Parsing collapsing option ..." )
parser.add_argument( "-i", action="store_true", dest="ite", default=False, help="enabling iterative collapsing (default=False)" )
args = parser.parse_args()
cur_path = os.getcwd()
f = open( cur_path + "/exp/benchmark/bench_list" )
content = f.readlines()
bench_list = [x.strip() for x in content]
for name in bench_list:
   bch = cur_path + ('/exp/benchmark/%s.blif' % name)
   if args.ite:
      log   = cur_path + ('/exp/log/%s_blif_i.log' % name)
      tcl   = cur_path + ('/exp/tcl/exp_blif_i.tcl')
      blif1 = cur_path + ('/exp/blif/%s_1_i.blif' % name) 
      blif2 = cur_path + ('/exp/blif/%s_2_i.blif' % name) 
   else:
      log   = cur_path + ('/exp/log/%s_blif.log' % name)
      tcl   = cur_path + ('/exp/tcl/exp_blif.tcl')
      blif1 = cur_path + ('/exp/blif/%s_1.blif' % name) 
      blif2 = cur_path + ('/exp/blif/%s_2.blif' % name) 
   cmd = cur_path + '/bin/abc -c \"r ' + bch + '; source ' + tcl + '\" &> ' + log
   sp.run(cmd, shell=True)
   cmd = 'mv compTH_1.blif ' + blif1
   sp.run(cmd, shell=True)
   cmd = 'mv compTH_2.blif ' + blif2
   sp.run(cmd, shell=True)
