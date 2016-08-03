#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import collections
import json


DEFAULT_PORT = '22'
DEFAULT_USER = 'chrome-bot'


SlaveInfo = collections.namedtuple('SlaveInfo',
                                   'ssh_user ssh_host ssh_port')

SLAVE_INFO = {
  'skiabot-shuttle-ubuntu12-003':
      SlaveInfo('root', '192.168.1.123', DEFAULT_PORT),
  'skiabot-shuttle-ubuntu12-004':
      SlaveInfo('root', '192.168.1.134', DEFAULT_PORT),
  'default':
      SlaveInfo('nouser', 'noip', 'noport'),
}


if __name__ == '__main__':
  print json.dumps(SLAVE_INFO)  # pragma: no cover

