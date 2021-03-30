# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api
import calendar

class PerfUploadApi(recipe_api.RecipeApi):
  def uploadNanoResults(self):
    # Upload the nanobench results.
    self.m.vars.setup()

    now = self.m.time.utcnow()
    #src_path = self.m.path['start_dir'].join('perf')
    src_path = self.m.flavor.host_dirs.perf_data_dir)
    with self.m.context(cwd=src_path):
      results = self.m.file.glob_paths(
          'find results',
          src_path,
          '*.json',
          test_data=['nanobench_abc123.json'])
    if len(results) != 1:  # pragma: nocover
      raise Exception('Unable to find nanobench or skpbench JSON file!')

    src = results[0]
    basename = self.m.path.basename(src)
    gs_path = '/'.join((
        'nano-json-v1', str(now.year).zfill(4),
        str(now.month).zfill(2), str(now.day).zfill(2), str(now.hour).zfill(2),
        self.m.vars.builder_name))

    if self.m.vars.is_trybot:
      gs_path = '/'.join(('trybot', gs_path,
                          str(self.m.vars.issue), str(self.m.vars.patchset)))

    dst = '/'.join((
        'gs://%s' % self.m.properties['gs_bucket'], gs_path, basename))

    self.m.gsutil.cp('upload', src, dst, extra_args=['-z', 'json'])
