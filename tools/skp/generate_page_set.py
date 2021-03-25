#!/usr/bin/env python
# Copyright (c) 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Script that generates a page_set for the webpages_playback.py script."""


from __future__ import print_function
import jinja2
import os


PAGE_SET_TEMPLATE = 'page_set_template'
PAGE_SET_DIR = 'page_sets'


def main():
  created_page_sets = []
  while True:
    user_agent = raw_input('user agent? (mobile/desktop/tablet): ')
    url_name = raw_input('URL name? (eg: google): ')
    url = raw_input('URL? (eg: http://www.google.com): ')
    comment = raw_input('Reason for adding the URL? (eg: go/skia-skps-3-2019): ')

    with open(PAGE_SET_TEMPLATE) as f:
      t = jinja2.Template(f.read())
    subs = {
      'user_agent': user_agent,
      'url_name': url_name,
      'url': url,
      'comment': comment,
    }

    page_set_name = 'skia_%s_%s.py' % (url_name, user_agent)
    page_set_path = os.path.join(PAGE_SET_DIR, page_set_name)
    with open(page_set_path, 'w') as f:
      f.write(t.render(**subs))
    created_page_sets.append(page_set_path)
    print('\nPage set has been created in %s\n\n' % page_set_path)

    keep_going = raw_input('Do you have more page sets to create? (y/n)')
    if keep_going != 'y':
      break

  print('\n\nSummarizing all created page sets:')
  for page_set_path in created_page_sets:
    print('* %s' % page_set_path)


if __name__ == '__main__':
  main()
