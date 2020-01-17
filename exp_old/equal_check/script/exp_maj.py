'''*************************************************************
FileName    [exp_maj.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script for MAJ EC experiment.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [9, August, 2018]
*******************************************************************'''

import os
import subprocess as sp
cur_path = os.getcwd()
f = open( cur_path + "/exp/benchmark/maj_list3" )
content = f.readlines()
bench_list = [x.strip() for x in content]
for name in bench_list:
   bch0      = cur_path + ('/exp/benchmark/%s.th' % name)
   bch1      = cur_path + ('/exp/benchmark/%s_v.th' % name)
   bch2      = cur_path + ('/exp/benchmark/%s_i.th' % name)
   log01_pb  = cur_path + ('/exp/log/%s_v_pb.log' % name)
   log01_cnf = cur_path + ('/exp/log/%s_v_cnf.log' % name)
   log02_pb  = cur_path + ('/exp/log/%s_i_pb.log' % name)
   log02_cnf = cur_path + ('/exp/log/%s_i_cnf.log' % name)
   opb01     = cur_path + ('/exp/opb/%s_v.opb' % name) 
   cnf01     = cur_path + ('/exp/dimacs/%s_v.dimacs' % name) 
   opb02     = cur_path + ('/exp/opb/%s_i.opb' % name) 
   cnf02     = cur_path + ('/exp/dimacs/%s_i.dimacs' % name) 
   # bch0 vs. bch1
   # pb
   cmd = cur_path + '/bin/abc -c \"thverify -V 0 ' + bch0 + ' ' + bch1 + '\"'
   sp.run(cmd, shell=True)
   cmd = 'mv compTH.opb ' + opb01
   sp.run(cmd, shell=True)
   cmd = 'timeout 7200 ' + cur_path + '/bin/minisat+ ' + opb01 + ' &> ' + log01_pb + ' &'
   sp.run(cmd, shell=True)
   # cnf
   cmd = cur_path + '/bin/abc -c \"thverify -V 1 ' + bch0 + ' ' + bch1 + '\"'
   sp.run(cmd, shell=True)
   cmd = 'mv compTH.dimacs ' + cnf01
   sp.run(cmd, shell=True)
   cmd = 'timeout 7200 ' + cur_path + '/bin/minisat ' + cnf01 + ' &> ' + log01_cnf + ' &'
   sp.run(cmd, shell=True)
   # bch0 vs. bch2
   # pb
   cmd = cur_path + '/bin/abc -c \"thverify -V 0 ' + bch0 + ' ' + bch2 + '\"'
   sp.run(cmd, shell=True)
   cmd = 'mv compTH.opb ' + opb02
   sp.run(cmd, shell=True)
   cmd = 'timeout 7200 ' + cur_path + '/bin/minisat+ ' + opb02 + ' &> ' + log02_pb + ' &'
   sp.run(cmd, shell=True)
   # cnf
   cmd = cur_path + '/bin/abc -c \"thverify -V 1 ' + bch0 + ' ' + bch2 + '\"'
   sp.run(cmd, shell=True)
   cmd = 'mv compTH.dimacs ' + cnf02
   sp.run(cmd, shell=True)
   cmd = 'timeout 7200 ' + cur_path + '/bin/minisat ' + cnf02 + ' &> ' + log02_cnf + ' &'
   sp.run(cmd, shell=True)
