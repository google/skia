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

def parse_fiddle_sk(x):
  class FiddleSk(HTMLParser):
    def __init__(self):
        HTMLParser.__init__(self)
        self.attrs = {}
    def handle_starttag(self, tag, attrs):
      if tag == 'fiddle-sk':
        self.attrs = dict(attrs)
  fiddle = FiddleSk()
  fiddle.feed(x)
  return fiddle.attrs

def process_fiddle(name):
  if name == 'MAD_Magazine_Oct_1985':
    return
  filename = 'docs/examples/%s.cpp' % name
  if os.path.exists(filename):
    return
  url = 'https://fiddle.skia.org/c/@' + name
  content = urlopen(url).read()
  regex = (r'(<fiddle-sk\s[^>]*>)\s*<textarea-numbers-sk>\s*'
           r'<textarea [^>]*>(.*)</textarea>')
  match = re.search(regex, content.decode('utf-8'), flags=re.S)
  if not match:
    sys.stderr.write('error: %s\n' % url)
  keys = parse_fiddle_sk(match.group(1))
  code = unescape(match.group(2))

  width = keys.get('width', '256')
  height = keys.get('height', '256')
  source_image = keys.get('source', 256)
  duration = keys.get('duration', '0')
  textonly = 'textonly' in keys
  srgb = not textonly and 'srgb' in keys
  f16 = srgb and 'f16' in keys
  offscreen = 'offscreen' in keys

  sys.stdout.write('Writing to: %s\n' % filename)
  sys.stdout.flush()
  with open(filename, 'w') as o:
    o.write('// Copyright 2020 Google LLC.\n'
            '// Use of this source code is governed by a BSD-style'
            ' license that can be found in the LICENSE file.\n'
            '#include "tools/fiddle/examples.h"\n')
    if offscreen:
      o.write('REGISTER_FIDDLE(')
      o.write(', '.join([name,
                         width,
                         height,
                         cxx_bool(textonly),
                         source_image,
                         duration,
                         cxx_bool(srgb),
                         cxx_bool(f16),
                         cxx_bool(offscreen),
                         keys.get('offscreen_width', '64'),
                         keys.get('offscreen_height', '64'),
                         keys.get('offscreen_sample_count', '0'),
                         keys.get('offscreen_texturable', 'false'),
                         keys.get('offscreen_mipmap', 'false')]))
    elif srgb:
      o.write('REG_FIDDLE_SRGB(')
      o.write(', '.join([name, width, height, cxx_bool(textonly),
                         source_image, duration, cxx_bool(f16)]))
    elif duration:
      o.write('REG_FIDDLE_ANIMATED(')
      o.write(', '.join([name, width, height, cxx_bool(textonly),
                         source_image, duration]))
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
