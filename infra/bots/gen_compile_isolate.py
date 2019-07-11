#!/usr/bin/env python
#
# Copyright 2019 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import difflib
import os
import re
import subprocess
import sys


INCLUDE_FILES = [
  r'.*\.cpp$',
  r'.*\.gn$',
  r'.*\.gni$',
  r'.*\.h$',
]

INCLUDE_EXPLICITLY = [
  '../.gclient',
  '.clang-format',
  '.clang-tidy',
  'bin/fetch-clang-format',
  'bin/fetch-gn',
  'buildtools/',
  'infra/canvaskit/',
  'infra/pathkit/',
  'resources/',
  'third_party/externals/',
]

COMBINE_DIRS_THRESHOLD = 3

NO_COMBINE_DIRS = [
  'infra',
]

ISOLATE_TMPL = '''{
  'includes': [
    'run_recipe.isolate',
  ],
  'variables': {
    'files': [
%s
    ],
  },
}'''

INFRABOTS_DIR = os.path.realpath(os.path.dirname(os.path.abspath(__file__)))
ISOLATE_FILE = os.path.join(INFRABOTS_DIR, 'compile.isolate')


def all_files():
  repo_root = os.path.abspath(os.path.join(INFRABOTS_DIR, os.pardir, os.pardir))
  output = subprocess.check_output(['git', 'ls-files'], cwd=repo_root).rstrip()
  return output.splitlines()


def get_relevant_files():
  files = []
  for f in all_files():
    for regexp in INCLUDE_FILES:
      if re.match(regexp, f):
        files.append(f)
        break

  files.extend(INCLUDE_EXPLICITLY)
  return files


class Tree(object):
  class Node(object):
    def __init__(self, name):
      self._children = {}
      self._name = name
      self._is_leaf = False

    @property
    def is_root(self):
      return self._name is None

    def add(self, entry):
      # Remove the first element if we're not the root node.
      if not self.is_root:
        entry = entry[1:]

        # If the entry is now empty, this node is a leaf.
        if len(entry) == 0:
          self._is_leaf = True
          return

      # Add a child node.
      child = self._children.get(entry[0])
      if not child:
        child = Tree.Node(entry[0])
        self._children[entry[0]] = child
      child.add(entry)

    def entries(self):
      if self._is_leaf:
        return [self._name]
      if not self.is_root and len(self._children) >= COMBINE_DIRS_THRESHOLD and not self._name in NO_COMBINE_DIRS:
        return [self._name]
      rv = []
      for child in self._children.itervalues():
        for entry in child.entries():
          if not self.is_root:
            entry = self._name + '/' + entry
          rv.append(entry)
      return rv

  def __init__(self):
    self._root = Tree.Node(None)

  def add(self, entry):
    split = entry.split('/')
    if split[-1] == '':
      split = split[:-1]
    self._root.add(split)

  def entries(self):
    return self._root.entries()


def relpath(repo_path):
  repo_path = '../../' + repo_path
  repo_path = repo_path.replace('../../infra/', '../')
  repo_path = repo_path.replace('../bots/', '')
  return repo_path


def get_isolate_content(files):
  lines = ['      \'%s\',' % relpath(f) for f in files]
  lines.sort()
  return ISOLATE_TMPL % '\n'.join(lines)


def main():
  testing = False
  if len(sys.argv) == 2 and sys.argv[1] == 'test':
    testing = True
  elif len(sys.argv) != 1:
    print >> sys.stderr, 'Usage: %s [test]' % sys.argv[0]
    sys.exit(1)

  tree = Tree()
  for f in get_relevant_files():
    tree.add(f)
  content = get_isolate_content(tree.entries())

  if testing:
    with open(ISOLATE_FILE, 'rb') as f:
      expect_content = f.read()
    if content != expect_content:
      print >> sys.stderr, 'Found diff in %s:' % ISOLATE_FILE
      a = expect_content.splitlines()
      b = content.splitlines()
      diff = difflib.context_diff(a, b, lineterm='')
      for line in diff:
        sys.stderr.write(line + '\n')
      print >> sys.stderr, 'You may need to run:\n\n\tpython %s' % sys.argv[0]
      sys.exit(1)
  else:
    with open(ISOLATE_FILE, 'wb') as f:
      f.write(content)


if __name__ == '__main__':
  main()
