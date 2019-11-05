# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'docker',
  'recipe_engine/context',
  'recipe_engine/step',
]


def RunSteps(api):
  api.docker.run(
      name='do Docker stuff',
      docker_image='my.docker.image',
      src_dir='/host-src',
      out_dir='/host-out',
      script='./do-stuff.sh',
      args=['--key', 'value'],
      docker_args=['--cpus', '2'],
      copies={'/copy-src/myfile': '/copy-dst/myfile'},
      recursive_read=['/host-src'],
  )

def GenTests(api):
  yield api.test('test')
