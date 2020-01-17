'''*************************************************************
FileName    [exp_pb_can.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for PB canonical EC experiment.]
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
      log  = cur_path + ('/exp/log/%s_pb_i_can.log' % name)
      log2 = cur_path + ('/exp/log/%s_pb_sol_i.log' % name)
      log3 = cur_path + ('/exp/log/%s_pb_sol_i_can.log' % name)
      tcl  = cur_path + ('/exp/tcl/exp_pb_i_can.tcl')
      opb  = cur_path + ('/exp/opb/%s_i.opb' % name)
      opb2 = cur_path + ('/exp/opb/%s_i_can.opb' % name)
   else:
      log  = cur_path + ('/exp/log/%s_pb_can.log' % name)
      log2 = cur_path + ('/exp/log/%s_pb_sol.log' % name)
      log3 = cur_path + ('/exp/log/%s_pb_sol_can.log' % name)
      tcl  = cur_path + ('/exp/tcl/exp_pb_can.tcl')
      opb  = cur_path + ('/exp/opb/%s.opb' % name) 
      opb2 = cur_path + ('/exp/opb/%s_can.opb' % name) 
   cmd = cur_path + '/bin/abc -c \"r ' + bch + '; source ' + tcl + '\" &> ' + log
   sp.run(cmd, shell=True)
   cmd = cur_path + '/bin/abc -c \"tvr compTH_1.th compTH_2.th; quit\" >> ' + log
   sp.run(cmd, shell=True)
   cmd = 'mv compTH.opb ' + opb
   sp.run(cmd, shell=True)
   cmd = 'timeout 7200 ' + cur_path + '/bin/minisat+ ' + opb + ' &> ' + log2 + ' &'
   sp.run(cmd, shell=True)
   cmd = cur_path + '/bin/abc_lp -c \"rt compTH_1.th; lp_con; wt compTH_1_can.th; quit\" >> ' + log
   sp.run(cmd, shell=True)
   cmd = cur_path + '/bin/abc_lp -c \"rt compTH_2.th; lp_con; wt compTH_2_can.th; quit\" >> ' + log
   sp.run(cmd, shell=True)
   cmd = cur_path + '/bin/abc -c \"tvr compTH_1_can.th compTH_2_can.th; quit\" >> ' + log
   sp.run(cmd, shell=True)
   cmd = 'mv compTH.opb ' + opb2
   sp.run(cmd, shell=True)
   cmd = 'timeout 7200 ' + cur_path + '/bin/minisat+ ' + opb2 + ' &> ' + log3 + ' &'
   sp.run(cmd, shell=True)
