'''*************************************************************
FileName    [maj.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script to generate majority functions.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [9, August, 2018]
*******************************************************************'''

import math
import argparse

def write_normal(f, n):
   f.write('Threshold logic gate list written by NZ.\n')
   f.write('.model maj_%d.th\n' % n)
   f.write('.input')
   for i in range(1, n+1):
      f.write(' %d' % i)
   f.write('\n')
   f.write('.output %d\n' % (n+1))
   f.write('.threshold %d %d\n' % (n+2, n+1))
   f.write('1 1\n')
   f.write('.threshold')
   for i in range(1, n+1):
      f.write(' %d' % i)
   f.write(' %d\n' % (n+2))
   for i in range(1, n+1):
      f.write('1 ')
   f.write('%d\n' % math.ceil(n/2))
   f.write('.end\n')
def write_var(f, n):
   k = int((n-1)*n/2 + 1)
   T = int(math.ceil(n/2))
   f.write('Threshold logic gate list written by NZ.\n')
   f.write('.model maj_%d_v.th\n' % n)
   f.write('.input')
   for i in range(1, n+1):
      f.write(' %d' % i)
   f.write('\n')
   f.write('.output %d\n' % (n+1))
   f.write('.threshold %d %d\n' % (n+2, n+1))
   f.write('1 1\n')
   f.write('.threshold')
   for i in range(1, n+1):
      f.write(' %d' % i)
   f.write(' %d\n' % (n+2))
   for i in range(0, n):
      f.write('%d ' % (k-i))
   f.write('%d\n' % (k*T-(k-1)))
   f.write('.end\n')
def write_ineq(f, n):
   f.write('Threshold logic gate list written by NZ.\n')
   f.write('.model maj_%d_i.th\n' % n)
   f.write('.input')
   for i in range(1, n+1):
      f.write(' %d' % i)
   f.write('\n')
   f.write('.output %d\n' % (n+1))
   f.write('.threshold %d %d\n' % (n+2, n+1))
   f.write('1 1\n')
   f.write('.threshold')
   for i in range(1, n+1):
      f.write(' %d' % i)
   f.write(' %d\n' % (n+2))
   for i in range(1, n+1):
      f.write('1 ')
   f.write('%d\n' % (math.ceil(n/2)-1) )
   f.write('.end\n')
def write_maj(f, t, n):
   if t == 0:
      write_normal(f, n)
   if t == 1:
      write_var(f, n)
   if t == 2:
      write_ineq(f, n)
parser = argparse.ArgumentParser( description = "Parsing # of input variables" )
parser.add_argument( "-n", action="store",  dest="n_input",  default=3, help="specifying # of input variables" )
parser.add_argument( "-t", action="store",  dest="maj_type",  default=0, help="specifying types of maj gates (0: normal, 1: var, 2: ineq)" )
args = parser.parse_args()
n = int(args.n_input)
t = int(args.maj_type)
if t == 0:
   file_name = 'maj_%d.th' % n
elif t == 1:
   file_name = 'maj_%d_v.th' % n
elif t == 2:
   file_name = 'maj_%d_i.th' % n
else:
   print('[ERROR] Unknown type %d!' % t)
   exit(1)
f = open(file_name, 'w')
write_maj(f, t, n)
f.close()
