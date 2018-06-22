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

  os = api.properties['os']
  compiler = api.properties['compiler']
  model = api.properties['model']
  cpu_or_gpu = api.properties['cpu_or_gpu']
  cpu_or_gpu_value = api.properties['cpu_or_gpu_value']
  arch = api.properties['arch']
  configuration = api.properties['configuration']
  test_filter = api.properties['test_filter']
  extra_tokens = api.properties.get('extra_tokens', '').split(',')
  api.flavor.setup(os, compiler, model, cpu_or_gpu, cpu_or_gpu_value, arch,
                   configuration, test_filter, extra_tokens)

  api.flavor.install(skps=True, svgs=True)
  api.file.ensure_directory('makedirs perf', api.vars.swarming_out_dir)

  skia_dir = api.path['start_dir'].join('skia')
  with api.context(cwd=skia_dir):
    extra_arg = '--svgs %s --skps %s' % (api.flavor.device_dirs.svg_dir,
                                         api.flavor.device_dirs.skp_dir)

    # measuring multi-picture-draw in our multi-threaded CPU test is inaccurate
    if api.properties['cpu_or_gpu'] == 'CPU':
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

    keys = ['os', 'compiler', 'model', 'cpu_or_gpu', 'cpu_or_gpu_value', 'arch',
            'extra_tokens']
    command.append('--keys')
    for k in sorted(keys):
      key = k
      value = api.properties[k]
      if key == 'extra_tokens':
        key = 'extra_config'
        value = '_'.join(value.split(','))
      if key and value:
        command.append(key)
        command.append(value)

    api.run(api.step, 'Run calmbench', cmd=command)
  api.run.check_failure()


TEST_BUILDERS = [
  'Calmbench-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All',
  'Calmbench-Ubuntu17-Clang-Golo-GPU-QuadroP400-x86_64-Release-All',
  'Calmbench-Ubuntu17-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-Bogus',
]


# Default properties used for TEST_BUILDERS.
def defaultProps(buildername):
  split = buildername.split('-')
  os = split[1]
  compiler = split[2]
  model = split[3]
  cpu_or_gpu = split[4]
  cpu_or_gpu_value = split[5]
  arch = split[6]
  configuration = split[7]
  test_filter = split[8]

  extra_tokens_list = []
  if len(split) > 9:
    extra_split = split[9].split('_')
    for idx, tok in enumerate(extra_split):
      if tok == 'SK':  # pragma: no cover
        extra_tokens_list.append('_'.join(extra_split[idx:]))
        break
      else:
        extra_tokens_list.append(tok)
  extra_tokens = ','.join(extra_tokens_list)

  return dict(
    arch=arch,
    buildername=buildername,
    buildbucket_build_id='123454321',
    compiler=compiler,
    configuration=configuration,
    cpu_or_gpu=cpu_or_gpu,
    cpu_or_gpu_value=cpu_or_gpu_value,
    extra_tokens=extra_tokens,
    model=model,
    os=os,
    revision='abc123',
    path_config='kitchen',
    swarm_out_dir='[SWARM_OUT_DIR]',
    test_filter=test_filter,
  )


def GenTests(api):
  for builder in TEST_BUILDERS:
    test = (
      api.test(builder) +
      api.properties(**defaultProps(builder)) +
      api.path.exists(
          api.path['start_dir'].join('skia'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
      )
    )

    yield test
