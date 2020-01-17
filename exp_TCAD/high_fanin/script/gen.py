'''*************************************************************
FileName    [gen.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script to generate high-fanin TL functions.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [12, June, 2019]
*******************************************************************'''

import math
import random
import argparse
import numpy as np

def is_not_constant(v):
   T = v[-1]
   M = 0
   m = 0
   for i in range(v.size):
      if v[i] > 0:
         M += v[i]
      else:
         m += v[i]
   if M < T or m >= T:
      return False
   else:
      return True
def generate_tl(n_input):
   v = np.zeros(n_input+1)
   while True:
      for i in range(v.size):
         v[i] = random.randint(-255, 255+1)
      if is_not_constant(v):
         return v
def write_tl_blif(f, v):
   n = v.size-1
   f.write('Threshold logic gate list written by NZ.\n')
   f.write('.model rand_%d.th\n' % n)
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
   for i in range(v.size):
      f.write('%d ' % v[i])
   f.write('\n.end')
a = [16, 20, 24, 28, 32, 36, 40]
N = 100
for n_input in a:
   for i in range(N):
      v = generate_tl(n_input)
      f = open('exp_TCAD/high_fanin/rand_tl/%d_%d.th' % (n_input, i), 'w')
      write_tl_blif(f, v)
      f.close()
