'''*************************************************************
FileName    [exp.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for EC experiment.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [1, August, 2018]
*******************************************************************'''

import os
import argparse
import subprocess as sp
parser = argparse.ArgumentParser( description = "Parsing experimental settings ..." )
parser.add_argument( "-i", action="store_true",  dest="ite",  default=False, help="enabling iterative collapsing (default=False)" )
parser.add_argument( "-a", action="store_false", dest="aig",  default=True,  help="performing aig-based EC  (default=True)" )
parser.add_argument( "-b", action="store_false", dest="blif", default=True,  help="performing blif-based EC (default=True)" )
parser.add_argument( "-c", action="store_false", dest="cnf",  default=True,  help="performing cnf-based EC  (default=True)" )
parser.add_argument( "-p", action="store_false", dest="pb",   default=True,  help="performing pb-based EC   (default=True)" )
args = parser.parse_args()
cur_path = os.getcwd()
if args.ite:
   if args.aig:
      cmd = 'python ' + cur_path + '/exp/script/exp_aig.py -i &'
      sp.run(cmd, shell=True)
   if args.blif:
      cmd = 'python ' + cur_path + '/exp/script/exp_blif.py -i &'
      sp.run(cmd, shell=True)
   if args.cnf:
      cmd = 'python ' + cur_path + '/exp/script/exp_cnf.py -i &'
      sp.run(cmd, shell=True)
   if args.pb:
      cmd = 'python ' + cur_path + '/exp/script/exp_pb.py -i &'
      sp.run(cmd, shell=True)
else:
   if args.aig:
      cmd = 'python ' + cur_path + '/exp/script/exp_aig.py &'
      sp.run(cmd, shell=True)
   if args.blif:
      cmd = 'python ' + cur_path + '/exp/script/exp_blif.py &'
      sp.run(cmd, shell=True)
   if args.cnf:
      cmd = 'python ' + cur_path + '/exp/script/exp_cnf.py &'
      sp.run(cmd, shell=True)
   if args.pb:
      cmd = 'python ' + cur_path + '/exp/script/exp_pb.py &'
      sp.run(cmd, shell=True)
