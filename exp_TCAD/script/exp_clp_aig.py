'''*************************************************************
FileName    [exp_clp_aig.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for collapsing AIG circuits experiments.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [16.01.2020]
*******************************************************************'''

import os
import argparse
import subprocess as sp
parser = argparse.ArgumentParser( description = "Parsing collapsing AIG circuits experimental settings ..." )
parser.add_argument( "-i", action="store_true",  dest="ite",  default=False, help="enabling iterative collapsing (default=False)" )
parser.add_argument( "-t", action="store_true",  dest="tcad", default=False, help="enabling TCAD suggestion (default=False)" )
args = parser.parse_args()
opt1 = " -B 100" if args.ite else ""
opt2 = " -t" if args.tcad else ""
syn  = "aig_syn; mt" + opt1 + opt2 + "; pt; q"
#cur_path = os.getcwd()
benchmarks = ["b14"]
for name in benchmarks:
   log = ("exp_TCAD/log/%s_aig_i.log" % name)
   tcl = "r exp_TCAD/benchmark/" + name + ".blif; " + syn
   cmd = "bin/abc -c \"" + tcl + "\" &> " + log
   sp.run(cmd, shell=True)
