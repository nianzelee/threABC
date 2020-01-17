'''*************************************************************
FileName    [exp_clp_tl.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for collapsing TL circuits experiments.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [16.01.2020]
*******************************************************************'''

import os
import glob
import argparse
import subprocess as sp
parser = argparse.ArgumentParser( description = "Parsing collapsing TL circuits experimental settings ..." )
parser.add_argument( "-i", action="store_true",  dest="ite",  default=False, help="enabling iterative collapsing (default=False)" )
parser.add_argument( "-t", action="store_true",  dest="tcad", default=False, help="enabling TCAD suggestion (default=False)" )
args = parser.parse_args()
opt1 = " -B 100" if args.ite else ""
opt2 = " -t" if args.tcad else ""
syn  = "tl_syn; mt" + opt1 + opt2 + "; pt; q"
suf1 = "_i" if args.ite else ""
suf2 = "_t" if args.tcad else ""
suf  = suf1 + suf2
for bch in glob.glob("exp_TCAD/collapse/benchmark/iscas_itc/*.blif"):
   name = os.path.basename(bch)
   log  = ("exp_TCAD/collapse/log/%s_tl%s.log" % (name, suf))
   tcl  = "r " + bch + "; " + syn
   cmd  = "bin/abc -c \"" + tcl + "\" &> " + log
   sp.run(cmd, shell=True)
