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


# Any files in Git which match these patterns will be included, either directly
# or indirectly via a parent dir.
PATH_PATTERNS = [
  r'.*\.c$',
  r'.*\.cc$',
  r'.*\.cpp$',
  r'.*\.gn$',
  r'.*\.gni$',
  r'.*\.h$',
  r'.*\.storyboard$',
]

# These paths are always added to the inclusion list. Note that they may not
# appear in the isolate if they are included indirectly via a parent dir.
EXPLICIT_PATHS = [
  '../.gclient',
  '.clang-format',
  '.clang-tidy',
  'bin/fetch-clang-format',
  'bin/fetch-gn',
  'buildtools',
  'infra/bots/assets/android_ndk_darwin/VERSION',
  'infra/bots/assets/android_ndk_linux/VERSION',
  'infra/bots/assets/android_ndk_windows/VERSION',
  'infra/bots/assets/cast_toolchain/VERSION',
  'infra/bots/assets/clang_linux/VERSION',
  'infra/bots/assets/clang_win/VERSION',
  'infra/bots/assets/mips64el_toolchain_linux/VERSION',
  'infra/canvaskit',
  'infra/pathkit',
  'resources',
  'third_party/externals',
]

# If a parent path contains more than this many immediate child paths (ie. files
# and dirs which are directly inside it as opposed to indirect descendants), we
# will include the parent in the isolate file instead of the children. This
# results in a simpler isolate file which should need to be changed less often.
COMBINE_PATHS_THRESHOLD = 3

# Template for the isolate file content.
ISOLATE_TMPL = '''{
  'includes': [
    'run_recipe.isolate',
  ],
  'variables': {
    'files': [
%s
    ],
  },
}
'''

# Absolute path to the infra/bots dir.
INFRABOTS_DIR = os.path.realpath(os.path.dirname(os.path.abspath(__file__)))

# Absolute path to the compile.isolate file.
ISOLATE_FILE = os.path.join(INFRABOTS_DIR, 'compile.isolate')


def all_paths():
  """Return all paths which are checked in to git."""
  repo_root = os.path.abspath(os.path.join(INFRABOTS_DIR, os.pardir, os.pardir))
  output = subprocess.check_output(['git', 'ls-files'], cwd=repo_root).rstrip()
  return output.splitlines()


def get_relevant_paths():
  """Return all checked-in paths in PATH_PATTERNS or EXPLICIT_PATHS."""
  paths = []
  for f in all_paths():
    for regexp in PATH_PATTERNS:
      if re.match(regexp, f):
        paths.append(f)
        break

  paths.extend(EXPLICIT_PATHS)
  return paths


class Tree(object):
  """Tree helps with deduplicating and collapsing paths."""
  class Node(object):
    """Node represents an individual node in a Tree."""
    def __init__(self, name):
      self._children = {}
      self._name = name
      self._is_leaf = False

    @property
    def is_root(self):
      """Return True iff this is the root node."""
      return self._name is None

    def add(self, entry):
      """Add the given entry (given as a list of strings) to the Node."""
      # Remove the first element if we're not the root node.
      if not self.is_root:
        if entry[0] != self._name:
          raise ValueError('Cannot add a non-matching entry to a Node!')
        entry = entry[1:]

        # If the entry is now empty, this node is a leaf.
        if not entry:
          self._is_leaf = True
          return

      # Add a child node.
      if not self._is_leaf:
        child = self._children.get(entry[0])
        if not child:
          child = Tree.Node(entry[0])
          self._children[entry[0]] = child
        child.add(entry)

        # If we have more than COMBINE_PATHS_THRESHOLD immediate children,
        # combine them into this node.
        immediate_children = 0
        for child in self._children.itervalues():
          if child._is_leaf:
            immediate_children += 1
        if not self.is_root and immediate_children >= COMBINE_PATHS_THRESHOLD:
          self._is_leaf = True
          self._children = {}

    def entries(self):
      """Return the entries represented by this node and its children.

      Will not return children in the following cases:
        - This Node is a leaf, ie. it represents an entry which was explicitly
          inserted into the Tree, as opposed to only part of a path to other
          entries.
        - This Node has immediate children exceeding COMBINE_PATHS_THRESHOLD and
          thus has been upgraded to a leaf node.
      """
      if self._is_leaf:
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
    """Add the given entry to the tree."""
    split = entry.split('/')
    if split[-1] == '':
      split = split[:-1]
    self._root.add(split)

  def entries(self):
    """Return the list of entries in the tree.

    Entries will be de-duplicated as follows:
      - Any entry which is a sub-path of another entry will not be returned.
      - Any entry which was not explicitly inserted but has children exceeding
        the COMBINE_PATHS_THRESHOLD will be returned while its children will not
        be returned.
    """
    return self._root.entries()


def relpath(repo_path):
  """Return a relative path to the given path within the repo.

  The path is relative to the infra/bots dir, where the compile.isolate file
  lives.
  """
  repo_path = '../../' + repo_path
  repo_path = repo_path.replace('../../infra/', '../')
  repo_path = repo_path.replace('../bots/', '')
  return repo_path


def get_isolate_content(paths):
  """Construct the new content of the isolate file based on the given paths."""
  lines = ['      \'%s\',' % relpath(p) for p in paths]
  lines.sort()
  return ISOLATE_TMPL % '\n'.join(lines)


def main():
  """Regenerate the compile.isolate file, or verify that it hasn't changed."""
  testing = False
  if len(sys.argv) == 2 and sys.argv[1] == 'test':
    testing = True
  elif len(sys.argv) != 1:
    print >> sys.stderr, 'Usage: %s [test]' % sys.argv[0]
    sys.exit(1)

  tree = Tree()
  for p in get_relevant_paths():
    tree.add(p)
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
