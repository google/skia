#!/usr/bin/env python

# Copyright 2016 The LUCI Authors. All rights reserved.
# Use of this source code is governed under the Apache License, Version 2.0
# that can be found in the LICENSE file.

"""Bootstrap script to clone and forward to the recipe engine tool.

***********************************************************************
** DO NOT MODIFY EXCEPT IN THE PER-REPO CONFIGURATION SECTION BELOW. **
***********************************************************************

This is a copy of https://github.com/luci/recipes-py/blob/master/doc/recipes.py.
To fix bugs, fix in the github repo then copy it back to here and fix the
PER-REPO CONFIGURATION section to look like this one.
"""

import os

#### PER-REPO CONFIGURATION (editable) ####
# The root of the repository relative to the directory of this file.
REPO_ROOT = os.path.join(os.pardir, os.pardir)
# The path of the recipes.cfg file relative to the root of the repository.
RECIPES_CFG = os.path.join('infra', 'config', 'recipes.cfg')
#### END PER-REPO CONFIGURATION ####

BOOTSTRAP_VERSION = 1

import argparse
import ast
import logging
import random
import re
import subprocess
import sys
import time
import traceback


def parse_protobuf(fh):
  """Parse the protobuf text format just well enough to understand recipes.cfg.

  We don't use the protobuf library because we want to be as self-contained
  as possible in this bootstrap, so it can be simply vendored into a client
  repo.

  We assume all fields are repeated since we don't have a proto spec to work
  with.

  Args:
    fh: a filehandle containing the text format protobuf.
  Returns:
    A recursive dictionary of lists.
  """
  def parse_atom(field, text):
    if text == 'true':
      return True
    if text == 'false':
      return False

    # repo_type is an enum. Since it does not have quotes,
    # invoking literal_eval would fail.
    if field == 'repo_type':
      return text

    return ast.literal_eval(text)

  ret = {}
  for line in fh:
    line = line.strip()
    m = re.match(r'(\w+)\s*:\s*(.*)', line)
    if m:
      ret.setdefault(m.group(1), []).append(parse_atom(m.group(1), m.group(2)))
      continue

    m = re.match(r'(\w+)\s*{', line)
    if m:
      subparse = parse_protobuf(fh)
      ret.setdefault(m.group(1), []).append(subparse)
      continue

    if line == '}':
      return ret
    if line == '':
      continue

    raise ValueError('Could not understand line: <%s>' % line)

  return ret


def get_unique(things):
  if len(things) == 1:
    return things[0]
  elif len(things) == 0:
    raise ValueError("Expected to get one thing, but dinna get none.")
  else:
    logging.warn('Expected to get one thing, but got a bunch: %s\n%s' %
                 (things, traceback.format_stack()))
    return things[0]


def _subprocess_call(argv, **kwargs):
  logging.info('Running %r', argv)
  return subprocess.call(argv, **kwargs)


def _subprocess_check_call(argv, **kwargs):
  logging.info('Running %r', argv)
  subprocess.check_call(argv, **kwargs)


def find_engine_override(argv):
  """Since the bootstrap process attempts to defer all logic to the recipes-py
  repo, we need to be aware if the user is overriding the recipe_engine
  dependency. This looks for and returns the overridden recipe_engine path, if
  any, or None if the user didn't override it."""
  PREFIX = 'recipe_engine='

  p = argparse.ArgumentParser()
  p.add_argument('-O', '--project-override', action='append')
  args, _ = p.parse_known_args(argv)
  for override in args.project_override or ():
    if override.startswith(PREFIX):
      return override[len(PREFIX):]
  return None


def main():
  if '--verbose' in sys.argv:
    logging.getLogger().setLevel(logging.INFO)

  if REPO_ROOT is None or RECIPES_CFG is None:
    logging.error(
      'In order to use this script, please copy it to your repo and '
      'replace the REPO_ROOT and RECIPES_CFG values with approprite paths.')
    sys.exit(1)

  if sys.platform.startswith(('win', 'cygwin')):
    git = 'git.bat'
  else:
    git = 'git'

  # Find the repository and config file to operate on.
  repo_root = os.path.abspath(
      os.path.join(os.path.dirname(__file__), REPO_ROOT))
  recipes_cfg_path = os.path.join(repo_root, RECIPES_CFG)

  with open(recipes_cfg_path, 'rU') as fh:
    protobuf = parse_protobuf(fh)

  engine_buf = get_unique([
      b for b in protobuf.get('deps', [])
      if b.get('project_id') == ['recipe_engine'] ])
  engine_url = get_unique(engine_buf['url'])
  engine_revision = get_unique(engine_buf['revision'])
  engine_subpath = (get_unique(engine_buf.get('path_override', ['']))
                    .replace('/', os.path.sep))

  recipes_path = os.path.join(repo_root,
      get_unique(protobuf.get('recipes_path', [''])).replace('/', os.path.sep))
  deps_path = os.path.join(recipes_path, '.recipe_deps')
  engine_path = find_engine_override(sys.argv[1:])
  if not engine_path:
    # Ensure that we have the recipe engine cloned.
    engine_path = os.path.join(deps_path, 'recipe_engine')
    def ensure_engine():
      if not os.path.exists(deps_path):
        os.makedirs(deps_path)
      if not os.path.exists(engine_path):
        _subprocess_check_call([git, 'clone', engine_url, engine_path])

      needs_fetch = _subprocess_call(
          [git, 'rev-parse', '--verify', '%s^{commit}' % engine_revision],
          cwd=engine_path, stdout=open(os.devnull, 'w'))
      if needs_fetch:
        _subprocess_check_call([git, 'fetch'], cwd=engine_path)
      _subprocess_check_call(
          [git, 'checkout', '--quiet', engine_revision], cwd=engine_path)

    try:
      ensure_engine()
    except subprocess.CalledProcessError:
      logging.exception('ensure_engine failed')

      # Retry errors.
      time.sleep(random.uniform(2,5))
      ensure_engine()

  args = ['--package', recipes_cfg_path] + sys.argv[1:]
  return _subprocess_call([
      sys.executable, '-u',
      os.path.join(engine_path, engine_subpath, 'recipes.py')] + args)

if __name__ == '__main__':
  sys.exit(main())
