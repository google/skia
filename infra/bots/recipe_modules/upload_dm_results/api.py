# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for uploading DM results.



import calendar

from recipe_engine import recipe_api


DM_JSON = 'dm.json'
GS_BUCKET = 'gs://skia-infra-gm'
UPLOAD_ATTEMPTS = 5
VERBOSE_LOG = 'verbose.log'


class UploadDmResultsApi(recipe_api.RecipeApi):
  def cp(self, name, src, dst, extra_args=None):
    cmd = ['gsutil', 'cp']
    if extra_args:
      cmd.extend(extra_args)
    cmd.extend([src, dst])

    name = 'upload %s' % name
    for i in xrange(UPLOAD_ATTEMPTS):
      step_name = name
      if i > 0:
        step_name += ' (attempt %d)' % (i+1)
      try:
        self.m.step(step_name, cmd=cmd)
        break
      except self.m.step.StepFailure:
        if i == UPLOAD_ATTEMPTS - 1:
          raise

  def run(self):
    builder_name = self.m.properties['buildername']
    revision = self.m.properties['revision']

    results_dir = self.m.path['start_dir'].join('dm')

    # Move dm.json and verbose.log to their own directory.
    json_file = results_dir.join(DM_JSON)
    log_file = results_dir.join(VERBOSE_LOG)
    tmp_dir = self.m.path['start_dir'].join('tmp_upload')
    self.m.shutil.makedirs('tmp dir', tmp_dir, infra_step=True)
    self.m.shutil.copy('copy dm.json', json_file, tmp_dir)
    self.m.shutil.copy('copy verbose.log', log_file, tmp_dir)
    self.m.shutil.remove('rm old dm.json', json_file)
    self.m.shutil.remove('rm old verbose.log', log_file)

    # Upload the images.
    image_dest_path = '/'.join((GS_BUCKET, 'dm-images-v1'))
    files_to_upload = self.m.file.glob(
        'find images',
        results_dir.join('*'),
        test_data=['someimage.png'],
        infra_step=True)
    if len(files_to_upload) > 0:
      self.cp('images', results_dir.join('*'), image_dest_path)

    # Upload the JSON summary and verbose.log.
    now = self.m.time.utcnow()
    summary_dest_path = '/'.join([
        'dm-json-v1',
        str(now.year ).zfill(4),
        str(now.month).zfill(2),
        str(now.day  ).zfill(2),
        str(now.hour ).zfill(2),
        revision,
        builder_name,
        str(int(calendar.timegm(now.utctimetuple())))])

    # Trybot results are further siloed by issue/patchset.
    issue = str(self.m.properties.get('issue', ''))
    patchset = str(self.m.properties.get('patchset', ''))
    if self.m.properties.get('patch_storage', '') == 'gerrit':
      issue = str(self.m.properties['patch_issue'])
      patchset = str(self.m.properties['patch_set'])
    if issue and patchset:
      summary_dest_path = '/'.join((
          'trybot', summary_dest_path, issue, patchset))

    summary_dest_path = '/'.join((GS_BUCKET, summary_dest_path))

    self.cp('JSON and logs', tmp_dir.join('*'), summary_dest_path,
       ['-z', 'json,log'])
