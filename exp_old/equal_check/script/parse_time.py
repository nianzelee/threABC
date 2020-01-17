'''*************************************************************
FileName    [parse_time.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for parsing time information.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [1, August, 2018]
*******************************************************************'''

import os
import argparse
import subprocess as sp
def cnf_get_time(file_name):
   cmd    = 'grep \"CPU time\" ' + file_name + ' | awk \'{print $4}\''
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   if output:
      return output
   else:
      return 'TO'
def pb_get_time(file_name):
   cmd    = 'grep \"CPU time\" ' + file_name + ' | awk \'{print $5}\''
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   if output:
      return output
   else:
      return 'TO'
def blif_get_time(file_name):
   cmd    = 'grep \"elapse\" ' + file_name + ' | tail -1 | awk \'{print $2}\''
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   if output:
      return output
   else:
      return 'TO'
def aig_get_time(file_name):
   cmd    = 'grep \"elapse\" ' + file_name + ' | tail -1 | awk \'{print $2}\''
   result = sp.check_output(cmd, shell=True)
   output = result.strip().decode()
   if output:
      return output
   else:
      return 'TO'
def write_time(csv, name, t_c, t_p, t_b, t_a):
   csv.write('%s,%s,%s,%s,%s\n' % (name, t_c, t_p, t_b, t_a))
parser = argparse.ArgumentParser(description = "Parsing collapsing option ...")
parser.add_argument( "-i", action="store_true", dest="ite", default=False, help="enabling iterative collapsing (default=False)" )
args = parser.parse_args()
cur_path = os.getcwd()
f = open(cur_path + '/exp/benchmark/bench_list')
content = f.readlines()
bench_list = [x.strip() for x in content]
if args.ite:
   csv = open(cur_path + '/exp/csv/time_i.csv', 'w')
   csv.write('benchmark,cnf,pb,blif,aig\n')
   for name in bench_list:
      t_c = cnf_get_time(cur_path + ('/exp/log/%s_cnf_sol_i.log' % name))
      t_p = pb_get_time(cur_path + ('/exp/log/%s_pb_sol_i.log' % name))
      t_b = blif_get_time(cur_path + ('/exp/log/%s_blif_i.log' % name))
      t_a = aig_get_time(cur_path + ('/exp/log/%s_aig_i.log' % name))
      write_time(csv, name, t_c, t_p, t_b, t_a)
else:
   csv = open(cur_path + '/exp/csv/time.csv', 'w')
   csv.write('benchmark,cnf,pb,blif,aig\n')
   for name in bench_list:
      t_c = cnf_get_time(cur_path + ('/exp/log/%s_cnf_sol.log' % name))
      t_p = pb_get_time(cur_path + ('/exp/log/%s_pb_sol.log' % name))
      t_b = blif_get_time(cur_path + ('/exp/log/%s_blif.log' % name))
      t_a = aig_get_time(cur_path + ('/exp/log/%s_aig.log' % name))
      write_time(csv, name, t_c, t_p, t_b, t_a)
csv.close()
