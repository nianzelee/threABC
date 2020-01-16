'''*************************************************************
FileName    [parse_size.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for parsing size information.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [1, August, 2018]
*******************************************************************'''

import os
import argparse
def write_size(csv, name, s_c, s_p, s_b, s_a):
   csv.write('%s,' % name)
   if s_c >= 1024000000:
      csv.write('MO,')
   else:
      csv.write('%.1f,' % (s_c/1000000))
   if s_p >= 1024000000:
      csv.write('MO,')
   else:
      csv.write('%.1f,' % (s_p/1000000))
   if s_b >= 1024000000:
      csv.write('MO,')
   else:
      csv.write('%.1f,' % (s_b/1000000))
   if s_a >= 1024000000:
      csv.write('MO\n')
   else:
      csv.write('%.1f\n' % (s_a/1000000))
parser = argparse.ArgumentParser( description = "Parsing collapsing option ..." )
parser.add_argument( "-i", action="store_true", dest="ite", default=False, help="enabling iterative collapsing (default=False)" )
args = parser.parse_args()
cur_path = os.getcwd()
f = open(cur_path + '/exp/benchmark/bench_list')
content = f.readlines()
bench_list = [x.strip() for x in content]
if args.ite:
   csv = open(cur_path + '/exp/csv/size_i.csv', 'w')
   csv.write('benchmark,cnf,pb,blif,aig\n')
   for name in bench_list:
      s_c = os.path.getsize(cur_path + ('/exp/dimacs/%s_i.dimacs' % name)) 
      s_p = os.path.getsize(cur_path + ('/exp/opb/%s_i.opb' % name)) 
      s_b = os.path.getsize(cur_path + ('/exp/blif/%s_1_i.blif' % name)) + os.path.getsize(cur_path + ('/exp/blif/%s_2_i.blif' % name))
      s_a = os.path.getsize(cur_path + ('/exp/aig/%s_1_i.aig' % name)) + os.path.getsize(cur_path + ('/exp/aig/%s_2_i.aig' % name))
      write_size(csv, name, s_c, s_p, s_b, s_a)
else:
   csv = open(cur_path + '/exp/csv/size.csv', 'w')
   csv.write('benchmark,cnf,pb,blif,aig\n')
   for name in bench_list:
      s_c = os.path.getsize(cur_path + ('/exp/dimacs/%s.dimacs' % name)) 
      s_p = os.path.getsize(cur_path + ('/exp/opb/%s.opb' % name)) 
      s_b = os.path.getsize(cur_path + ('/exp/blif/%s_1.blif' % name)) + os.path.getsize(cur_path + ('/exp/blif/%s_2.blif' % name))
      s_a = os.path.getsize(cur_path + ('/exp/aig/%s_1.aig' % name)) + os.path.getsize(cur_path + ('/exp/aig/%s_2.aig' % name))
      write_size(csv, name, s_c, s_p, s_b, s_a)
csv.close()
