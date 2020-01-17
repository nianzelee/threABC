'''*************************************************************
FileName    [main.py]
ProjectName [Threshold logic synthesis and verification.]
Synopsis    [Script to generate high-fanin TL functions.]
Author      [Nian-Ze Lee]
Affiliation [NTU]
Date        [10, June, 2019]
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
def quantize_tl(v):
   u = np.copy(v)
   for i in range(u.size):
      u[i] = int(u[i]/16)
   return u
def generate_nn(n_input, n_layer):
   N = []
   for i in range(n_input*n_layer):
      N.append(generate_tl(n_input))
   return N
def quantize_nn(N):
   M = list(N)
   for i in range(len(M)):
      M[i] = quantize_tl(M[i])
   return M
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
def write_nn_blif(f, N, n_input, n_layer):
   f.write('Threshold logic gate list written by NZ.\n')
   f.write('.model rand_%d_%d.th\n' % (n_input, n_layer))
   f.write('.input')
   for i in range(1, n_input+1):
      f.write(' %d' % i)
   f.write('\n')
   f.write('.output')
   for i in range(1, n_input+1):
      f.write(' %d' % ((n_layer+1)*n_input+i))
   f.write('\n')
   for i in range(1, n_input+1):
      f.write('.threshold %d %d\n' % (n_layer*n_input+i, (n_layer+1)*n_input+i))
      f.write('1 1\n')
   for j in range(1, n_layer+1):
      for i in range(1, n_input+1):
         v = N[(j-1)*n_layer+i-1]
         v_id = j*n_input+i
         f.write('.threshold')
         for k in range(1, n_input+1):
            f.write(' %d' % ((j-1)*n_input+k))
         f.write(' %d\n' % v_id)
         for k in range(v.size):
            f.write('%d ' % v[k])
         f.write('\n')
   f.write('.end')
'''
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
'''
'''
a = [16, 20, 24, 28, 32, 36, 40, 64, 128, 256]
for n_input in a:
   v = generate_tl(n_input)
   #u = generate_tl(n_input)
   u = quantize_tl(v)
   f = open('rand_%d.th' % n_input, 'w')
   g = open('rand_%d_q.th' % n_input, 'w')
   #f = open('rand_%d_1.th' % n_input, 'w')
   #g = open('rand_%d_2.th' % n_input, 'w')
   write_tl_blif(f, v)
   write_tl_blif(g, u)
   f.close()
   g.close()
'''
a = [32, 48, 64, 128]
b = [3, 4, 5]
for n_input in a:
   for n_layer in b:
      N = generate_nn(n_input, n_layer)
      M = quantize_nn(N)
      f = open('rand_nn/%d_%d.th' % (n_input, n_layer), 'w')
      g = open('rand_nn/%d_%d_q.th' % (n_input, n_layer), 'w')
      write_nn_blif(f, N, n_input, n_layer)
      write_nn_blif(g, M, n_input, n_layer)
      f.close()
      g.close()
