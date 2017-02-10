#!/usr/bin/env python
import os
import re
import subprocess
import sys

logs = sys.argv[1]
stack_line = r'^(?P<path>.+)\(\+?(?P<addr>.*)\) \[(?P<addr2>.+)\]'
found_stack = False
for line in logs.splitlines():
  line = line.strip()
  if not found_stack and line != 'Stack trace:':
    continue
  found_stack = True

  m = re.search(stack_line, line)
  if m:
    #print m.group('path'), m.group('addr'), m.group('addr2')
    path = m.group('path')
    addr = m.group('addr')
    addr2 = m.group('addr2')
    if os.path.exists(path):
      if not addr or not addr.startswith('0x'):
        addr = addr2
      sym = subprocess.check_output(['addr2line', '-Cfpe', path, addr])
      # print 'addr2line', '-Cfpe', path, addr
      # sym = "SomeFunction() ;asdifweiksfd: 48"
      line = path + ' ' + sym.strip()
  # if len(tokens) == 11 and tokens[-7] == 'F' and tokens[-3] == 'pc':
  #   addr, path = tokens[-2:]
  #   local = os.path.join(out, os.path.basename(path))
  #   if os.path.exists(local):
  #     #sym = subprocess.check_output(['addr2line', '-Cfpe', local, addr])
  #     print 'addr2line', '-Cfpe', local, addr
  #     sym = "FOO"
  #     line = line.replace(addr, addr + ' ' + sym.strip())
  print line