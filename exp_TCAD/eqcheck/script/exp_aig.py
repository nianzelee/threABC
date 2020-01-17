'''*************************************************************
FileName    [exp_aig.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for AIG EC experiment.]
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
f = open( cur_path + "/exp_TCAD/eqcheck/benchmark/bench_list" )
content = f.readlines()
bench_list = [x.strip() for x in content]
for name in bench_list:
   bch = cur_path + ('/exp_TCAD/eqcheck/benchmark/%s.blif' % name)
   if args.ite:
      log  = cur_path + ('/exp_TCAD/eqcheck/log/%s_aig_i.log' % name)
      tcl  = cur_path + ('/exp_TCAD/eqcheck/tcl/exp_aig_i.tcl')
      aig1 = cur_path + ('/exp_TCAD/eqcheck/aig/%s_1_i.aig' % name) 
      aig2 = cur_path + ('/exp_TCAD/eqcheck/aig/%s_2_i.aig' % name) 
   else:
      log  = cur_path + ('/exp_TCAD/eqcheck/log/%s_aig.log' % name)
      tcl  = cur_path + ('/exp_TCAD/eqcheck/tcl/exp_aig.tcl')
      aig1 = cur_path + ('/exp_TCAD/eqcheck/aig/%s_1.aig' % name) 
      aig2 = cur_path + ('/exp_TCAD/eqcheck/aig/%s_2.aig' % name) 
   cmd = cur_path + '/bin/abc -c \"r ' + bch + '; source ' + tcl + '\" &> ' + log
   sp.run(cmd, shell=True)
   cmd = 'mv compTH_1.aig ' + aig1
   sp.run(cmd, shell=True)
   cmd = 'mv compTH_2.aig ' + aig2
   sp.run(cmd, shell=True)
