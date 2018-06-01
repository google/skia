# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming calmbench.

DEPS = [
  'flavor',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'recipe_engine/time',
  'run',
  'vars',
]

def RunSteps(api):
  api.vars.setup()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)
  api.flavor.setup()

  api.flavor.install(skps=True, svgs=True)
  api.file.ensure_directory('makedirs perf', api.vars.swarming_out_dir)

  skia_dir = api.path['start_dir'].join('skia')
  with api.context(cwd=skia_dir):
    extra_arg = '--svgs %s --skps %s' % (api.flavor.device_dirs.svg_dir,
                                         api.flavor.device_dirs.skp_dir)

    # measuring multi-picture-draw in our multi-threaded CPU test is inaccurate
    if api.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
      extra_arg += ' --mpd false'
      config = "8888"
    else:
      config = "gl"

    command = [
        'python',
        skia_dir.join('tools', 'calmbench', 'ab.py'),
        api.vars.swarming_out_dir,
        'modified', 'master',
        api.vars.build_dir.join('nanobench'),
        api.vars.build_dir.join('ParentRevision', 'nanobench'),
        extra_arg, extra_arg,
        2,          # reps
        "false",    # skipbase
        config,
        -1,         # threads; let ab.py decide the threads
        "false",    # noinit
        "--githash", api.properties['revision'],
        "--concise"
    ]

    keys_blacklist = ['configuration', 'role', 'test_filter']
    command.append('--keys')
    for k in sorted(api.vars.builder_cfg.keys()):
      if not k in keys_blacklist:
        command.extend([k, api.vars.builder_cfg[k]])

    api.run(api.step, 'Run calmbench', cmd=command)
  api.run.check_failure()

def GenTests(api):
  builders = [
    "Calmbench-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All",
    "Calmbench-Ubuntu17-Clang-Golo-GPU-QuadroP400-x86_64-Release-All",
  ]

  for builder in builders:
    test = (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(
          api.path['start_dir'].join('skia'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
      )
    )

    yield test
