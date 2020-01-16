#! /usr/bin/env python
# Copyright 2020 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
get_examples.py: Populate docs/examples/ from the list of named fiddles.
'''

import os
import re
import sys

if sys.version_info[0] < 3:
  from urllib2 import urlopen
  from HTMLParser import HTMLParser
  def unescape(v): return HTMLParser().unescape(v)
else:
  from urllib.request import urlopen
  from html.parser import HTMLParser
  from html import unescape

def cxx_bool(v): return 'true' if v else 'false'

assert os.pardir == '..'  and '/' in [os.sep, os.altsep]

def process_fiddle(name):
  if name == 'MAD_Magazine_Oct_1985':
    return
  filename = 'docs/examples/%s.cpp' % name
  if os.path.exists(filename):
    return
  url = 'https://fiddle.skia.org/c/@' + name
  content = urlopen(url).read()
  regex = ('<fiddle-sk display_options bug_link '
           'width="([0-9]*)" height="([0-9]*)" source="([0-9]*)" '
           '(.*)<textarea [^>]*>(.*)</textarea>')
  match = re.search(regex, content.decode('utf-8'), flags=re.S)
  if not match:
    sys.stderr.write('error: %s\n' % url)

  width = match.group(1)
  height = match.group(2)
  source_image = match.group(3)
  code = unescape(match.group(5))
  extra = re.sub('\s+', ' ', match.group(4))
  textonly = 'textonly' in extra

  mduration = re.search(' duration="([0-9]+)" ', extra)
  duration = int(mduration.group(1)) if mduration else 0

  skip = False
  for term in ['offscreen_texturable', 'offscreen_mipmap', 'srgb']:
    if ' ' + term + ' ' in extra:
      sys.stdout.write('SKIPPING [%s]: %s\n' % (term, name))
      skip = True
  if skip:
    sys.stdout.flush()
    return

  sys.stdout.write('Writing to: %s\n' % filename)
  sys.stdout.flush()
  with open(filename, 'w') as o:
    o.write('// Copyright 2020 Google LLC.\n'
            '// Use of this source code is governed by a BSD-style'
            ' license that can be found in the LICENSE file.\n'
            '#include "tools/fiddle/examples.h"\n')
    if duration:
      o.write('REG_FIDDLE_ANIMATED(')
      o.write(', '.join([name, width, height, cxx_bool(textonly),
                         source_image, str(duration)]))
    else:
      o.write('REG_FIDDLE(')
      o.write(', '.join([name, width, height, cxx_bool(textonly),
                         source_image]))
    o.write(') {\n')
    o.write(code)
    o.write('\n}  // END FIDDLE\n')

def main():
  os.chdir(os.path.dirname(__file__) + '/../..')
  for line in urlopen('https://fiddle.skia.org/named/'):
    line_match = re.search(r'/c/@([A-Za-z0-9_-]*)', line.decode('utf-8'))
    if not line_match:
      continue
    name = line_match.group(1)
    process_fiddle(name)

if __name__ == '__main__':
  main()
