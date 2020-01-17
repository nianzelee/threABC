'''*************************************************************
FileName    [exp_pg.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for PG encoding experiment.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [17, January, 2020]
*******************************************************************'''

import os
import argparse
import numpy as np
import pandas as pd
import subprocess as sp
def pb_get_time(file_name):
   cmd    = 'grep \"CPU time\" ' + file_name + ' | awk \'{print $5}\''
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   if output:
      return output
   else:
      return 'TO'
parser = argparse.ArgumentParser( description = "Parsing PG encoding experimental settings ..." )
parser.add_argument( "-a", action="store_true",  dest="conj",  default=False, help="Conjoining all POs (default=False)" )
args  = parser.parse_args()
pg    = "pg_and" if args.conj else "pg_or"
suf   = "andpos" if args.conj else "orpos"
csv   = "exp_TCAD/pg_encoding/csv/" + suf + ".csv"
bench = ['b14', 'b15', 'b17', 'b18', 'b19', 'b20', 'b21', 'b22']
col   = ['c_npg', 'c_pg', 's_npg', 's_pg', 't_npg', 't_pg']
df    = pd.DataFrame(np.zeros(shape=(len(bench), len(col))), index=bench, columns=col)
for bch in bench:
   read_path = 'exp_TCAD/pg_encoding/benchmark/%s.blif' % bch
   cmd = './bin/abc -c \"r ' + read_path + '; ' + pg + "; q\""
   sp.run(cmd, shell=True)
   # no_pg
   opb = "exp_TCAD/pg_encoding/opb/" + bch + "_no_pg_" + suf + ".opb"
   cmd = "mv no_pg.opb " + opb
   sp.run(cmd, shell=True) 
   df.at[bch, 'c_npg'] = sum(1 for line in open(opb))-1
   df.at[bch, 's_npg'] = os.path.getsize(opb)
   log = "exp_TCAD/pg_encoding/log/" + bch + "_no_pg_" + suf + ".log"
   cmd = './bin/minisat+ ' + opb + " > " + log
   sp.run(cmd, shell=True)
   df.at[bch, 't_npg'] = pb_get_time(log)
   # pg
   opb = "exp_TCAD/pg_encoding/opb/" + bch + "_pg_" + suf + ".opb"
   cmd = "mv pg.opb " + opb
   sp.run(cmd, shell=True) 
   df.at[bch, 'c_pg'] = sum(1 for line in open(opb))-1
   df.at[bch, 's_pg'] = os.path.getsize(opb)
   log = "exp_TCAD/pg_encoding/log/" + bch + "_pg_" + suf + ".log"
   cmd = './bin/minisat+ ' + opb + " > " + log
   sp.run(cmd, shell=True)
   df.at[bch, 't_pg'] = pb_get_time(log)
df.to_csv(csv)
