# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api
import calendar

DM_JSON = 'dm.json'

class GoldUploadApi(recipe_api.RecipeApi):
  def upload(self):
    """Attempt to upload files to Gold.
    This module assumes setup has occurred for the vars and flavor modules.
    """
    revision = self.m.properties['revision']
    results_dir = self.m.flavor.host_dirs.dm_dir

    # Upload the images. It is preferred that the images are uploaded first
    # so they exist whenever the json is processed.
    image_dest_path = 'gs://%s/dm-images-v1' % self.m.properties['gs_bucket']
    for ext in ['.png']:
      files_to_upload = self.m.file.glob_paths(
          'find %s images' % ext,
          results_dir,
          '*%s' % ext,
          test_data=['someimage.png'])
      # For some reason, glob returns results_dir when it should return nothing.
      files_to_upload = [f for f in files_to_upload if str(f).endswith(ext)]
      if len(files_to_upload) > 0:
        self.m.gsutil.cp('%s images' % ext, results_dir.join('*%s' % ext),
                        image_dest_path, multithread=True)

    summary_dest_path = 'gs://%s' % self.m.properties['gs_bucket']
    ref = revision
    # Trybot results are siloed by issue/patchset.
    if self.m.vars.is_trybot:
      summary_dest_path = '/'.join([summary_dest_path, 'trybot'])
      ref = '%s_%s' % (str(self.m.vars.issue), str(self.m.vars.patchset))

    # Compute the directory to upload results to
    now = self.m.time.utcnow()
    summary_dest_path = '/'.join([
        summary_dest_path,
        'dm-json-v1',
        str(now.year ).zfill(4),
        str(now.month).zfill(2),
        str(now.day  ).zfill(2),
        str(now.hour ).zfill(2),
        ref,
        self.m.vars.builder_name,
        str(int(calendar.timegm(now.utctimetuple())))])

    # Directly upload dm.json if it exists.
    json_file = results_dir.join(DM_JSON)
    # -Z compresses the json file at rest with gzip.
    self.m.gsutil.cp('dm.json', json_file,
                  summary_dest_path + '/' + DM_JSON, extra_args=['-Z'])

