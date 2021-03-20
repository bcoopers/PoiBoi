import os
import subprocess

def compile_poiboi():
  hdrs = []
  srcs = []

  cc_dir = './cc_src/'
  for fname in os.listdir(cc_dir):
    if fname.endswith('_test.cc'):
      continue
    if fname.endswith('.h'):
      hdrs.append(cc_dir + fname)
    elif fname.endswith('.cc'):
      srcs.append(cc_dir + fname)
  subprocess.run(['g++', '-std=c++17', '-O2', '-Wall'] + srcs + ['-o', 'poiboi.exe'])

compile_poiboi()
