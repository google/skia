#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import datetime


class print_timings(object):
  def __init__(self):
    self._start = None

  def __enter__(self):
    self._start = datetime.datetime.utcnow()
    print 'Task started at %s GMT' % str(self._start)

  def __exit__(self, t, v, tb):
    finish = datetime.datetime.utcnow()
    duration = (finish-self._start).total_seconds()
    print 'Task finished at %s GMT (%f seconds)' % (str(finish), duration)
