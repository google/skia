#!/usr/bin/env python
# Copyright (c) 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Script that generates a page_set for the webpages_playback.py script."""

import jinja2
import sys


PAGE_SET_TEMPLATE = 'page_set_template'
PAGE_SET_DIR = 'page_set'


# TODO(Rmistry):
# * raw_inputs
# * write to file
# * test it manually with mobile, tablet, desktop.

if '__main__' == __name__:
  print 'here here'
  with open(PAGE_SET_TEMPLATE) as f:
    t = jinja2.Template(f.read())
  subs = {
    'user_agent': 'mobile',
    'url_name': 'amazon',
    'url': 'http://www.amazon.com',
    'comment': 'go/skia-skps-3-2019',
  }

  dest = ''
  print t.render(**subs)


  sys.exit()
