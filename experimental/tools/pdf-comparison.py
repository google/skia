#!/usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
This tool compares the PDF output of Skia's DM tool of two commits.

It relies on pdfium_test being in the PATH.  To build:

mkdir -p ~/src/pdfium
cd ~/src/pdfium
gclient config --unmanaged https://pdfium.googlesource.com/pdfium.git
gclient sync
cd pdfium
gn gen out/default --args='pdf_enable_xfa=false pdf_enable_v8=false pdf_is_standalone=true'
ninja -C out/default pdfium_test
cp out/default/pdfium_test ~/bin/
'''

import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading

EXTRA_GN_ARGS = os.environ.get('PDF_COMPARISON_GN_ARGS', '')

REFERENCE_BACKEND = 'gl' if 'PDF_COMPARISON_NOGPU' not in os.environ else '8888'

DPI = float(os.environ.get('PDF_COMPARISON_DPI', 72))

PDF_CONFIG = 'pdf' if 'PDF_COMPARISON_300DPI' not in os.environ else 'pdf300'

BAD_TESTS = [
  'image-cacherator-from-picture',
  'image-cacherator-from-raster',
  'mixershader',
  'shadermaskfilter_image',
  'tilemode_decal',
]

NINJA = 'ninja'

PDFIUM_TEST = 'pdfium_test'

NUM_THREADS = int(os.environ.get('PDF_COMPARISON_THREADS', 40))

SOURCES = ['gm']

def test_exe(cmd):
  with open(os.devnull, 'w') as o:
    try:
      subprocess.call([cmd], stdout=o, stderr=o)
    except OSError:
      return False
  return True

def print_cmd(cmd, o):
  m = re.compile('[^A-Za-z0-9_./-]')
  o.write('+ ')
  for c in cmd:
    if m.search(c) is not None:
      o.write(repr(c) + ' ')
    else:
      o.write(c + ' ')
  o.write('\n')
  o.flush()

def check_call(cmd, **kwargs):
  print_cmd(cmd, sys.stdout)
  return subprocess.check_call(cmd, **kwargs)

def check_output(cmd, **kwargs):
  print_cmd(cmd, sys.stdout)
  return subprocess.check_output(cmd, **kwargs)

def remove(*paths):
  for path in paths:
    os.remove(path)

def timeout(deadline, cmd):
  #print_cmd(cmd, sys.stdout)
  with open(os.devnull, 'w') as o:
    proc = subprocess.Popen(cmd, stdout=o, stderr=subprocess.STDOUT)
    timer = threading.Timer(deadline, proc.terminate)
    timer.start()
    proc.wait()
    timer.cancel()
    return proc.returncode

def is_same(path1, path2):
  if not os.path.isfile(path1) or not os.path.isfile(path2):
    return os.path.isfile(path1) == os.path.isfile(path2)
  with open(path1, 'rb') as f1:
    with open(path2, 'rb') as f2:
      while True:
        c1, c2 = f1.read(4096), f2.read(4096)
        if c1 != c2:
          return False
        if not c1:
          return True


def getfilesoftype(directory, ending):
  for dirpath, _, filenames in os.walk(directory):
    rp = os.path.normpath(os.path.relpath(dirpath, directory))
    for f in filenames:
      if f.endswith(ending):
        yield os.path.join(rp, f)

def get_common_paths(dirs, ext):
  return sorted(list(
    set.intersection(*(set(getfilesoftype(d, ext)) for d in dirs))))

def printable_path(d):
  if 'TMPDIR' in os.environ:
    return d.replace(os.path.normpath(os.environ['TMPDIR']) + '/', '$TMPDIR/')
  return d

def spawn(cmd):
  with open(os.devnull, 'w') as o:
    subprocess.Popen(cmd, stdout=o, stderr=o)

def sysopen(arg):
  plat = sys.platform
  if plat.startswith('darwin'):
    spawn(["open", arg])
  elif plat.startswith('win'):
    # pylint: disable=no-member
    os.startfile(arg)
  else:
    spawn(["xdg-open", arg])

HTML_HEAD = '''
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>DIFF</title>
<style>
body{
background-size:16px 16px;
background-color:rgb(230,230,230);
background-image:
linear-gradient(45deg,rgba(255,255,255,.2) 25%,transparent 25%,transparent 50%,
rgba(255,255,255,.2) 50%,rgba(255,255,255,.2) 75%,transparent 75%,transparent)}
div.r{position:relative;left:0;top:0}
table{table-layout:fixed;width:100%}
img.s{max-width:100%;max-height:320;left:0;top:0}
img.b{position:absolute;mix-blend-mode:difference}
</style>
<script>
function r(c,e,n,g){
t=document.getElementById("t");
function ce(t){return document.createElement(t);}
function ct(n){return document.createTextNode(n);}
function ac(u,v){u.appendChild(v);}
function cn(u,v){u.className=v;}
function it(s){ td=ce("td"); a=ce("a"); a.href=s; img=ce("img"); img.src=s;
        cn(img,"s"); ac(a,img); ac(td,a); return td; }
tr=ce("tr"); td=ce("td"); td.colSpan="4"; ac(td, ct(n)); ac(tr,td);
ac(t,tr); tr=ce("tr"); td=ce("td"); dv=ce("div"); cn(dv,"r");
img=ce("img"); img.src=c; cn(img,"s"); ac(dv,img); img=ce("img");
img.src=e; cn(img,"s b"); ac(dv,img); ac(td,dv); ac(tr,td);
ac(tr,it(c)); ac(tr,it(e)); ac(tr,it(g)); ac(t,tr); }
document.addEventListener('DOMContentLoaded',function(){
'''

HTML_TAIL = '''];
for(i=0;i<z.length;i++){
r(c+z[i][0],e+z[i][0],z[i][2],c+z[i][1]);}},false);
</script></head><body><table id="t">
<tr><th>BEFORE-AFTER DIFF</th>
<th>BEFORE</th><th>AFTER</th>
<th>REFERENCE</th></tr>
</table></body></html>'''

def shard(fn, arglist):
  jobs = [[arg for j, arg in enumerate(arglist) if j % NUM_THREADS == i]
          for i in range(NUM_THREADS)]
  results = []
  def do_shard(*args):
    for arg in args:
      results.append(fn(arg))
  thread_list = []
  for job in jobs:
    t = threading.Thread(target=do_shard, args=job)
    t.start()
    thread_list += [t]
  for t in thread_list:
    t.join()
  return results

def shardsum(fn, arglist):
  'return the number of True results returned by fn(arg) for arg in arglist.'
  return sum(1 for result in shard(fn, arglist) if result)

def checkout_worktree(checkoutable):
  directory = os.path.join(tempfile.gettempdir(), 'skpdf_control_tree')
  commit = check_output(['git', 'rev-parse', checkoutable]).strip()
  if os.path.isdir(directory):
    try:
      check_call(['git', 'checkout', commit], cwd=directory)
      return directory
    except subprocess.CalledProcessError:
      shutil.rmtree(directory)
  check_call(['git', 'worktree', 'add', '-f', directory, commit])
  return directory

def build_skia(directory, executable):
  args = ('--args=is_debug=false'
          ' extra_cflags=["-DSK_PDF_LESS_COMPRESSION",'
          ' "-DSK_PDF_BASE85_BINARY"] ')
  if test_exe('ccache'):
    args += ' cc_wrapper="ccache"'
  args += EXTRA_GN_ARGS
  build_dir = directory + '/out/pdftest'
  check_call([sys.executable, 'bin/sync'], cwd=directory)
  check_call([directory + '/bin/gn', 'gen', 'out/pdftest', args],
             cwd=directory)
  check_call([NINJA, executable], cwd=build_dir)
  return os.path.join(build_dir, executable)

def build_and_run_dm(directory, data_dir):
  dm = build_skia(directory, 'dm')
  for source in SOURCES:
    os.makedirs(os.path.join(data_dir, PDF_CONFIG, source))
  dm_args = [dm, '--src'] + SOURCES + ['--config', PDF_CONFIG, '-w', data_dir]
  if BAD_TESTS:
    dm_args += ['-m'] + ['~^%s$' % x for x in BAD_TESTS]
  check_call(dm_args, cwd=directory)
  return dm

def rasterize(path):
  ret = timeout(30, [PDFIUM_TEST, '--png', '--scale=%g' % (DPI / 72.0), path])
  if ret != 0:
    sys.stdout.write(
      '\nTIMEOUT OR ERROR [%d] "%s"\n' % (ret, printable_path(path)))
    return
  assert os.path.isfile(path + '.0.png')

def main(control_commitish):
  assert os.pardir == '..'  and '/' in [os.sep, os.altsep]
  assert test_exe(NINJA)
  assert test_exe(PDFIUM_TEST)
  os.chdir(os.path.dirname(__file__) + '/../..')
  control_worktree = checkout_worktree(control_commitish)
  tmpdir = tempfile.mkdtemp(prefix='skpdf_')
  exp = tmpdir + '/experim'
  con = tmpdir + '/control'
  build_and_run_dm(os.curdir, exp)
  dm = build_and_run_dm(control_worktree, con)
  image_diff_metric = build_skia(control_worktree, 'image_diff_metric')

  out = sys.stdout
  common_paths = get_common_paths([con, exp], '.pdf')
  out.write('\nNumber of PDFs: %d\n\n' % len(common_paths))
  def compare_identical(path):
    cpath, epath = (os.path.join(x, path) for x in (con, exp))
    if is_same(cpath, epath):
      remove(cpath, epath)
      return True
    return False
  identical_count = shardsum(compare_identical, common_paths)
  out.write('Number of identical PDFs: %d\n\n' % identical_count)

  differing_paths = get_common_paths([con, exp], '.pdf')
  if not differing_paths:
    out.write('All PDFs are the same!\n')
    sys.exit(0)
  out.write('Number of differing PDFs: %d\n' % len(differing_paths))
  for p in differing_paths:
    out.write('  %s\n' % printable_path(tmpdir + '/*/' + p))
  out.write('\n')
  shard(rasterize,
        [os.path.join(x, p) for p in differing_paths for x in [con, exp]])

  common_pngs = get_common_paths([con, exp], '.pdf.0.png')
  identical_count = shardsum(compare_identical, common_pngs)
  out.write('Number of PDFs that rasterize the same: %d\n\n'
            % identical_count)

  differing_pngs = get_common_paths([con, exp], '.pdf.0.png')
  if not differing_pngs:
    out.write('All PDFs rasterize the same!\n')
    sys.exit(0)
  out.write('Number of PDFs that rasterize differently: %d\n'
            % len(differing_pngs))
  for p in differing_pngs:
    out.write('  %s\n' % printable_path(tmpdir + '/*/' + p))
  out.write('\n')

  scores = dict()
  def compare_differing_pngs(path):
    cpath, epath = (os.path.join(x, path) for x in (con, exp))
    s = float(subprocess.check_output([image_diff_metric, cpath, epath]))
    indicator = '.' if s < 0.001 else ':' if s < 0.01 else '!'
    sys.stdout.write(indicator)
    sys.stdout.flush()
    scores[path] = s
  shard(compare_differing_pngs, differing_pngs)
  paths = sorted(scores.iterkeys(), key=lambda p: -scores[p])
  out.write('\n\n')
  for p in paths:
    pdfpath = printable_path(tmpdir + '/*/' + p.replace('.0.png', ''))
    out.write('  %6.4f  %s\n' % (scores[p], pdfpath))
  out.write('\n')

  errors = []
  rc = re.compile('^' + PDF_CONFIG + r'/([^/]*)/([^/]*)\.pdf\.0\.png$')
  for p in paths:
    m = rc.match(p)
    assert(m)
    source, name = m.groups()
    errors.append((source, name, scores[p]))

  for source in SOURCES:
    os.makedirs(os.path.join(con, REFERENCE_BACKEND, source))
  dm_args = [dm, '--src'] + SOURCES + [
             '--config', REFERENCE_BACKEND, '-w', con, '-m'] + [
             '^%s$' % name for _, name, _ in errors]
  check_call(dm_args, cwd=control_worktree)

  report = tmpdir + '/report.html'
  with open(report, 'w') as o:
    o.write(HTML_HEAD)
    o.write('c="%s/";\n' % os.path.relpath(con, tmpdir))
    o.write('e="%s/";\n' % os.path.relpath(exp, tmpdir))
    o.write('z=[\n')
    for source, name, score in errors:
      gt = REFERENCE_BACKEND + '/' + source + '/' + name + '.png'
      p = '%s/%s/%s.pdf.0.png' % (PDF_CONFIG, source, name)
      desc = '%s | %s | %g' % (source, name, score)
      o.write('["%s","%s","%s"],\n' % (p, gt, desc))
    o.write(HTML_TAIL)
  out.write(printable_path(report) + '\n')
  sysopen(report)

if __name__ == '__main__':
  if len(sys.argv) != 2:
    USAGE = ('\nusage:\n  {0} COMMIT_OR_BRANCH_TO_COMPARE_TO\n\n'
             'e.g.:\n  {0} HEAD\nor\n  {0} HEAD~1\n\n')
    sys.stderr.write(USAGE.format(sys.argv[0]))
    sys.exit(1)
  main(sys.argv[1])
