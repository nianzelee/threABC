'''*************************************************************
FileName    [parse_maj.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for parsing MAJ exp information.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [9, August, 2018]
*******************************************************************'''

import os
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
def write_info(csv, name, s_p, t_p, s_c, t_c):
   csv.write('%s,%s,%s,%s,%s\n' % (name, s_p, t_p, s_c/1000000, t_c))
cur_path = os.getcwd()
f = open(cur_path + '/exp/benchmark/maj_list3')
content = f.readlines()
bench_list = [x.strip() for x in content]
csv = open(cur_path + '/exp/csv/maj_v.csv', 'w')
csv.write('benchmark,s_pb(B),t_pb(sec),s_cnf(MB),t_cnf(sec)\n')
for name in bench_list:
   s_p = os.path.getsize(cur_path + ('/exp/opb/%s_v.opb' % name)) 
   t_p = pb_get_time(cur_path + ('/exp/log/%s_v_pb.log' % name)) 
   s_c = os.path.getsize(cur_path + ('/exp/dimacs/%s_v.dimacs' % name)) 
   t_c = cnf_get_time(cur_path + ('/exp/log/%s_v_cnf.log' % name))
   write_info(csv, name, s_p, t_p, s_c, t_c)
csv.close()
csv = open(cur_path + '/exp/csv/maj_i.csv', 'w')
csv.write('benchmark,s_pb(B),t_pb(sec),s_cnf(MB),t_cnf(sec)\n')
for name in bench_list:
   s_p = os.path.getsize(cur_path + ('/exp/opb/%s_i.opb' % name)) 
   t_p = pb_get_time(cur_path + ('/exp/log/%s_i_pb.log' % name)) 
   s_c = os.path.getsize(cur_path + ('/exp/dimacs/%s_i.dimacs' % name)) 
   t_c = cnf_get_time(cur_path + ('/exp/log/%s_i_cnf.log' % name))
   write_info(csv, name, s_p, t_p, s_c, t_c)
csv.close()
