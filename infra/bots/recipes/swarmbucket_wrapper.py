# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe wrapper used in SwarmBucket.


DEPS = [
  'build/file',
  'depot_tools/bot_update',
  'depot_tools/gclient',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
]


def checkout_steps(api):
  """Run the steps to obtain a checkout of Skia."""
  # Find the workdir and cache dir.
  workdir = api.path['b'].join('work')
  if not api.path.exists(workdir):
    api.file.makedirs('workdir', workdir, infra_step=True)
  cache_dir = api.path['b'].join('cache')

  # Set up gclient config.
  api.gclient.use_mirror = True
  gclient_cfg = api.gclient.make_config(GIT_MODE=True, CACHE_DIR=cache_dir)
  soln = gclient_cfg.solutions.add()
  soln.name = 'skia'
  soln.url = 'https://skia.googlesource.com/skia.git'
  soln.revision = api.properties.get('revision', 'origin/master')
  api.gclient.c = gclient_cfg
  api.gclient.c.got_revision_mapping['skia'] = 'got_revision'

  # Set up options for bot_update based on properties. In general, these will
  # all take the default value.
  patch = api.properties.get('patch', True)
  clobber = True if api.properties.get('clobber') else False
  no_shallow = True if api.properties.get('no_shallow') else False
  output_manifest = api.properties.get('output_manifest', False)
  with_branch_heads = api.properties.get('with_branch_heads', False)
  refs = api.properties.get('refs', [])
  oauth2 = api.properties.get('oauth2', False)
  root_solution_revision = api.properties.get('root_solution_revision')
  suffix = api.properties.get('suffix')
  gerrit_no_reset = True if api.properties.get('gerrit_no_reset') else False

  # Run bot_update to sync the code and apply a patch if necessary.
  api.bot_update.ensure_checkout(no_shallow=no_shallow,
                                 patch=patch,
                                 with_branch_heads=with_branch_heads,
                                 output_manifest=output_manifest,
                                 refs=refs, patch_oauth2=oauth2,
                                 clobber=clobber,
                                 root_solution_revision=root_solution_revision,
                                 suffix=suffix,
                                 gerrit_no_reset=gerrit_no_reset,
                                 cwd=workdir)

  # Ensure that we ended up with the desired revision.
  got_revision = api.step.active_result.presentation.properties['got_revision']
  if soln.revision != 'origin/master':  # pragma: no cover
    assert got_revision == soln.revision
  return got_revision


def forward_to_recipe_in_repo(api):
  workdir = api.path['b'].join('work')
  recipes_py = workdir.join('skia', 'infra', 'bots', 'recipes.py')
  cmd = ['python', recipes_py, 'run',
         '--workdir', workdir,
         'swarm_trigger', 'path_config=kitchen']
  for k, v in api.properties.iteritems():
    cmd.append('%s=%s' % (k, v))
  api.step('run recipe', cmd=cmd, allow_subannotations=True)


def print_properties(api):
  """Dump out all properties for debugging purposes."""
  props = {}
  for k, v in api.properties.iteritems():
    props[k] = v
  api.python.inline(
      'print properties',
      '''
import json
import sys

with open(sys.argv[1]) as f:
  content = json.load(f)

print json.dumps(content, indent=2)
''',
      args=[api.json.input(props)])


def RunSteps(api):
  api.path.c.base_paths['b'] = ('/', 'b')

  # TODO(borenet): Remove this once SwarmBucket is working.
  print_properties(api)

  checkout_steps(api)
  forward_to_recipe_in_repo(api)


def GenTests(api):
  yield (
      api.test('trigger_recipe') +
      api.properties(buildername='Some-Builder',
                     buildnumber=5,
                     mastername='client.skia.fyi',
                     slavename='some-slave',
                     path_config='swarmbucket')
  )
