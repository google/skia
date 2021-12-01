# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = "PY3"

DEPS = [
  'docker',
  'recipe_engine/context',
  'recipe_engine/properties',
  'recipe_engine/step',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  api.docker.run(
      name='do Docker stuff',
      docker_image='my.docker.image',
      src_dir='/host-src',
      out_dir='/host-out',
      script='./do-stuff.sh',
      args=['--src', api.docker.mount_src(), '--out', api.docker.mount_out()],
      docker_args=['--cpus', '2'],
      copies=[{'src': '/copy-src/myfile', 'dst': '/copy-dst/myfile'}],
      recursive_read=['/host-src'],
  )

def GenTests(api):
  yield (api.test('test') +
         api.properties(buildername='Test-Debian10-EMCC-GCE-GPU-WEBGL1-wasm-Debug-All-CanvasKit',
                        buildbucket_build_id='123454321',
                        revision='abc123',
                        path_config='kitchen',
                        gold_hashes_url='https://example.com/hashes.txt',
                        swarm_out_dir='[SWARM_OUT_DIR]',
                        task_id='task_12345')
  )
