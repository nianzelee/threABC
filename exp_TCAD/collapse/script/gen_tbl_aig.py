'''*************************************************************
FileName    [gen_tbl_aig.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for generating tables for AIG circuits experiments.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [16.01.2020]
*******************************************************************'''

import os
import glob
import argparse
import subprocess as sp

def get_pi_num(log):
   cmd = "grep -m 1 \'PI\' " + log + " | awk \'{print $3}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def get_po_num(log):
   cmd = "grep -m 1 \'PO\' " + log + " | awk \'{print $3}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def get_aig_num(log):
   cmd = "grep -m 1 \'Node\' " + log + " | awk \'{print $3}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def get_lva_num(log):
   cmd = "grep -m 1 \'Level\' " + log + " | awk \'{print $3}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def get_tlg_num(log):
   cmd = "grep \'Node\' " + log + " | tail -1 | awk \'{print $3}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def get_lvt_num(log):
   cmd = "grep \'Level\' " + log + " | tail -1 | awk \'{print $3}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def get_time(log):
   cmd = "grep \'time\' " + log + " | awk \'{print $5}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def write_bch_info(tbl, name, log):
  pi   = get_pi_num(log)
  po   = get_po_num(log)
  aig  = get_aig_num(log)
  lva  = get_lva_num(log)
  tlg  = get_tlg_num(log)
  lvt  = get_lvt_num(log)
  time = get_time(log)
  tbl.write("%s,%s,%s,%s,%s,%s,%s,%s\n" % (name,pi,po,aig,lva,tlg,lvt,time))

parser = argparse.ArgumentParser( description = "Parsing collapsing AIG circuits experimental settings ..." )
parser.add_argument( "-i", action="store_true",  dest="ite",  default=False, help="enabling iterative collapsing (default=False)" )
parser.add_argument( "-t", action="store_true",  dest="tcad", default=False, help="enabling TCAD suggestion (default=False)" )
args = parser.parse_args()
suf1 = "_i" if args.ite else ""
suf2 = "_t" if args.tcad else ""
suf  = suf1 + suf2
tbl  = open(("exp_TCAD/collapse/tbl/tbl_clp_aig%s.csv" % suf), 'w')
tbl.write('benchmark,#PI,#PO,#AIG,#level,TLG,#level,time(s)\n')
for bch in glob.glob("exp_TCAD/collapse/benchmark/iscas_itc/*.blif"):
   name = os.path.basename(bch)
   log  = ("exp_TCAD/collapse/log/%s_aig%s.log" % (name, suf))
   write_bch_info(tbl, name, log)
