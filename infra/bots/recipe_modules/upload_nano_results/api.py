# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for uploading nanobench results.


from recipe_engine import recipe_api


class UploadNanoResultsApi(recipe_api.RecipeApi):
  def run(self):
    # Upload the nanobench resuls.
    builder_name = self.m.properties['buildername']

    now = self.m.time.utcnow()
    src_path = self.m.path['start_dir'].join(
        'perfdata', builder_name, 'data')
    results = self.m.file.glob(
        'find results',
        '*.json',
        cwd=src_path,
        test_data=['nanobench_abc123.json'],
        infra_step=True)
    if len(results) != 1:  # pragma: nocover
      raise Exception('Unable to find nanobench or skpbench JSON file!')

    src = src_path.join(results[0])
    basename = self.m.path.basename(src)
    gs_path = '/'.join((
        'nano-json-v1', str(now.year).zfill(4),
        str(now.month).zfill(2), str(now.day).zfill(2), str(now.hour).zfill(2),
        builder_name))

    issue = str(self.m.properties.get('issue', ''))
    patchset = str(self.m.properties.get('patchset', ''))
    if self.m.properties.get('patch_storage', '') == 'gerrit':
      issue = str(self.m.properties['patch_issue'])
      patchset = str(self.m.properties['patch_set'])
    if issue and patchset:
      gs_path = '/'.join(('trybot', gs_path, issue, patchset))

    dst = '/'.join(('gs://skia-perf', gs_path, basename))

    self.m.step(
        'upload',
        cmd=['gsutil', 'cp', '-a', 'public-read', '-z', 'json', src, dst],
        infra_step=True)
