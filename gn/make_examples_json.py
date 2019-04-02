#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import glob
import json
import os
if __name__ == '__main__':
    os.chdir(os.path.dirname(__file__))
    with open('examples.json', 'w') as o:
        json.dump(sorted(glob.glob('../docs/examples/*.cpp')), o,
                  indent=2, separators=(',', ': '))
        o.write('\n')
