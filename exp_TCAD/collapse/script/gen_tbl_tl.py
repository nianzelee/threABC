'''*************************************************************
FileName    [gen_tbl_tl.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for generating tables for TL circuits experiments.]
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
def get_tlg1_num(log):
   cmd = "grep -m 1 \'Node\' " + log + " | awk \'{print $3}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def get_lv1_num(log):
   cmd = "grep -m 1 \'Level\' " + log + " | awk \'{print $3}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def get_time1(log):
   cmd = "grep \'elapse\' " + log + " | tail -1 | awk \'{print $2}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def get_tlg2_num(log):
   cmd = "grep \'Node\' " + log + " | tail -1 | awk \'{print $3}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def get_lv2_num(log):
   cmd = "grep \'Level\' " + log + " | tail -1 | awk \'{print $3}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def get_time2(log):
   cmd = "grep \'time\' " + log + " | awk \'{print $5}\'"
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   return output
def write_bch_info(tbl, name, log):
  pi   = get_pi_num(log)
  po   = get_po_num(log)
  tlg1 = get_tlg1_num(log)
  lv1  = get_lv1_num(log)
  t1   = get_time1(log)
  tlg2 = get_tlg2_num(log)
  lv2  = get_lv2_num(log)
  t2   = get_time2(log)
  tbl.write("%s,%s,%s,%s,%s,%s,%s,%s,%s\n" % (name,pi,po,tlg1,lv1,t1,tlg2,lv2,t2))

parser = argparse.ArgumentParser( description = "Parsing collapsing TL circuits experimental settings ..." )
parser.add_argument( "-i", action="store_true",  dest="ite",  default=False, help="enabling iterative collapsing (default=False)" )
parser.add_argument( "-t", action="store_true",  dest="tcad", default=False, help="enabling TCAD suggestion (default=False)" )
args = parser.parse_args()
suf1 = "_i" if args.ite else ""
suf2 = "_t" if args.tcad else ""
suf  = suf1 + suf2
tbl  = open(("exp_TCAD/collapse/tbl/tbl_clp_tl%s.csv" % suf), 'w')
tbl.write('benchmark,#PI,#PO,#TLG,#level,time(s),#TLG,#level,time(s)\n')
for bch in glob.glob("exp_TCAD/collapse/benchmark/iscas_itc/*.blif"):
   name = os.path.basename(bch)
   log  = ("exp_TCAD/collapse/log/%s_tl%s.log" % (name, suf))
   write_bch_info(tbl, name, log)
