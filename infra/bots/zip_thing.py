import os
import sys
import zip_utils


files = [
  os.path.join('path', 'to', 'file'),
  os.path.join('toplevel_file'),
]


target_dir = sys.argv[1]
for f in files:
  d, base = os.path.split(f)
  if d:
    os.makedirs(os.path.join(target_dir, d))
  with open(os.path.join(target_dir, f), 'wb') as fp:
    fp.write('ksdaklasdklasdfklasdfkl')


zip_file = 'test.zip'
zip_utils.zip(target_dir, zip_file)
