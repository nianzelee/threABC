'''*************************************************************
FileName    [exp_pb.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for PB EC experiment.]
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
      log  = cur_path + ('/exp_TCAD/eqcheck/log/%s_pb_i.log' % name)
      log2 = cur_path + ('/exp_TCAD/eqcheck/log/%s_pb_sol_i.log' % name)
      tcl  = cur_path + ('/exp_TCAD/eqcheck/tcl/exp_pb_i.tcl')
      opb  = cur_path + ('/exp_TCAD/eqcheck/opb/%s_i.opb' % name)
   else:
      log  = cur_path + ('/exp_TCAD/eqcheck/log/%s_pb.log' % name)
      log2 = cur_path + ('/exp_TCAD/eqcheck/log/%s_pb_sol.log' % name)
      tcl  = cur_path + ('/exp_TCAD/eqcheck/tcl/exp_pb.tcl')
      opb  = cur_path + ('/exp_TCAD/eqcheck/opb/%s.opb' % name) 
   cmd = cur_path + '/bin/abc -c \"r ' + bch + '; source ' + tcl + '\" &> ' + log
   sp.run(cmd, shell=True)
   cmd = 'mv compTH.opb ' + opb
   sp.run(cmd, shell=True)
   cmd = 'timeout 7200 ' + cur_path + '/bin/minisat+ ' + opb + ' &> ' + log2 + ' &'
   sp.run(cmd, shell=True)
