# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# TODO(borenet): This module belongs in the recipe engine. Remove it from this
# repo once it has been moved.


from recipe_engine import recipe_api


class FileApi(recipe_api.RecipeApi):
  """FileApi contains helper functions for reading and writing files."""

  def __init__(self, **kwargs):
    super(FileApi, self).__init__(**kwargs)

  def _run_fileutil(self, name, fileutil_args, **kwargs):
    # Failure to perform filesystem operations is considered an infrastructure
    # failure.
    kwargs = kwargs.copy()
    kwargs.setdefault('infra_step', True)

    self.m.python(
        name,
        self.resource('fileutil.py'),
        args=fileutil_args,
        **kwargs)

  def copy(self, name, source, dest, step_test_data=None, **kwargs):
    """Copy a file."""
    return self.m.python.inline(
        name,
        """
        import shutil
        import sys
        shutil.copy(sys.argv[1], sys.argv[2])
        """,
        args=[source, dest],
        add_python_log=False,
        step_test_data=step_test_data,
        **kwargs
    )

  def copytree(self, name, source, dest, symlinks=False, **kwargs):
    """Run shutil.copytree in a step."""
    return self.m.python.inline(
        name,
        """
        import shutil
        import sys
        shutil.copytree(sys.argv[1], sys.argv[2], symlinks=bool(sys.argv[3]))
        """,
        args=[source, dest, int(symlinks)],
        add_python_log=False,
        **kwargs
    )

  def move(self, name, source, dest, **kwargs):
    """Run shutil.move in a step."""
    return self.m.python.inline(
        name,
        """
        import shutil
        import sys
        shutil.move(sys.argv[1], sys.argv[2])
        """,
        args=[source, dest],
        add_python_log=False,
        **kwargs
    )

  def read(self, name, path, test_data=None, **kwargs):
    """Read a file and return its contents."""
    step_test_data = None
    if test_data is not None:
      step_test_data = lambda: self.m.raw_io.test_api.output_text(test_data)
    return self.copy(name, path, self.m.raw_io.output_text(),
                     step_test_data=step_test_data, **kwargs).raw_io.output_text

  def write(self, name, path, data, **kwargs):
    """Write the given data to a file."""
    return self.m.python.inline(
        name,
        """
        import shutil
        import sys
        shutil.copy(sys.argv[1], sys.argv[2])
        """,
        args=[self.m.raw_io.input_text(data), path],
        add_python_log=False,
        **kwargs
    )

  def glob(self, name, pattern, test_data=None, **kwargs):
    """Performs glob search on a directory.

    Returns list of Path objects for all files found.
    """
    step_test_data = None
    if test_data is not None:
      step_test_data = (
          lambda: self.m.raw_io.test_api.output_text(
              '\n'.join(map(str, test_data))))
    step_result = self.m.python.inline(
        name,
        r"""
        import glob
        import sys
        with open(sys.argv[1], 'w') as f:
          f.write('\n'.join(glob.glob(sys.argv[2])))
        """,
        args=[self.m.raw_io.output_text(), pattern],
        step_test_data=step_test_data,
        add_python_log=False,
        **kwargs
    )
    return map(self.m.path.abs_to_path,
               step_result.raw_io.output_text.splitlines())

  def remove(self, name, path, **kwargs):
    """Remove the given file."""
    return self.m.python.inline(
        name,
        """
        import os
        import sys
        os.remove(sys.argv[1])
        """,
        args=[path],
        **kwargs
    )

  def listdir(self, name, path, step_test_data=None, **kwargs):
    """Wrapper for os.listdir."""
    return self.m.python.inline('listdir %s' % name,
      """
      import json, os, sys
      if os.path.exists(sys.argv[1]) and os.path.isdir(sys.argv[1]):
        with open(sys.argv[2], 'w') as f:
          json.dump(os.listdir(sys.argv[1]), f)
      """,
      args=[path, self.m.json.output()],
      step_test_data=(step_test_data or (
          lambda: self.m.json.test_api.output(['file 1', 'file 2']))),
      **kwargs
    ).json.output

  def makedirs(self, name, path, mode=0777, **kwargs):
    """
    Like os.makedirs, except that if the directory exists, then there is no
    error.
    """
    self.m.path.assert_absolute(path)
    self.m.python.inline(
      'makedirs ' + name,
      """
      import sys, os
      path = sys.argv[1]
      mode = int(sys.argv[2])
      if not os.path.isdir(path):
        if os.path.exists(path):
          print "%s exists but is not a dir" % path
          sys.exit(1)
        os.makedirs(path, mode)
      """,
      args=[path, str(mode)],
      **kwargs
    )
    self.m.path.mock_add_paths(path)

  def rmtree(self, name, path, **kwargs):
    """Wrapper for chromium_utils.RemoveDirectory."""
    self.m.path.assert_absolute(path)
    self._run_fileutil(
        'rmtree ' + name,
        ['rmtree', path],
        **kwargs)

  def rmcontents(self, name, path, **kwargs):
    """
    Similar to rmtree, but removes only contents not the directory.

    This is useful e.g. when removing contents of current working directory.
    Deleting current working directory makes all further getcwd calls fail
    until chdir is called. chdir would be tricky in recipes, so we provide
    a call that doesn't delete the directory itself.
    """
    self.m.path.assert_absolute(path)
    self._run_fileutil(
        'rmcontents ' + name,
        ['rmcontents', path],
        **kwargs)

  def rmwildcard(self, pattern, path, **kwargs):
    """
    Removes all files in the subtree of path matching the glob pattern.
    """
    self.m.path.assert_absolute(path)
    self._run_fileutil(
        'rmwildcard %s in %s' % (pattern, path),
        ['rmwildcard', path, pattern],
        **kwargs)
