#! /usr/bin/env python

# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import csv
import os
import shutil
import sys

def gset(path):
    s = set()
    if os.path.isfile(path):
        with open(path, 'r') as f:
            for line in f:
                s.add(line.strip())
    return s

def main():
    assert '/' in [os.sep, os.altsep]
    assets = os.path.join(os.path.dirname(__file__), os.pardir, os.pardir,
                          'platform_tools/android/apps/skqp/src/main/assets')
    models = gset(assets + '/gmkb/models.txt')
    good = gset('good.txt')
    bad = gset('bad.txt')
    assert good.isdisjoint(bad)
    do_score = good & models
    no_score = bad | (good - models)
    to_delete = models & bad
    for d in to_delete:
        path = assets + '/gmkb/' + d
        if os.path.isdir(path):
            shutil.rmtree(path)
    results = dict()
    for n in do_score:
        results[n] = 0
    for n in no_score:
        results[n] = -1
    skqp =  assets + '/skqp'
    if not os.path.isdir(skqp):
        os.mkdir(skqp)
    with open(skqp + '/rendertests.txt', 'w') as o:
        for n in sorted(results):
            o.write('%s,%d\n' % (n, results[n]))

if __name__ == '__main__':
    main()

