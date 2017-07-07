# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api

import re


class CTApi(recipe_api.RecipeApi):
  """Provides steps to run CT tasks."""

  CT_GS_BUCKET = 'cluster-telemetry'

  def download_swarming_skps(self, page_type, slave_num, skps_chromium_build,
                             dest_dir, start_range, num_skps):
    """Downloads SKPs corresponding to the specified page type, slave and build.

    The SKPs are stored in Google Storage in the following dirs in CT_GS_BUCKET:
      /swarming/skps/${page_type}/${skps_chromium_build}/{start_range..end_num}/
    The SKPs are downloaded into subdirectories in the dest_dir.

    Args:
      api: RecipeApi instance.
      page_type: str. The CT page type. Eg: 1k, 10k.
      slave_num: int. The number of the swarming bot.
      skps_chromium_build: str. The build the SKPs were captured from.
      dest_dir: path obj. The directory to download SKPs into.
      start_range: int. The subdirectory number to start from.
      num_skps: int. The total number of SKPs to download starting with
                     start_range.
    """
    slave_dest_dir = dest_dir.join('slave%s' % slave_num )
    remote_dir = 'gs://%s/swarming/skps/%s/%s' % (
        self.CT_GS_BUCKET, page_type, skps_chromium_build)

    # Delete and recreate the local dir.
    self.m.run.rmtree(slave_dest_dir)
    self.m.file.ensure_directory(
      'makedirs %s' % self.m.path.basename(slave_dest_dir), slave_dest_dir)

    # Populate the empty local dir.
    gsutil_args = ['-m', 'cp']
    for i in range(start_range, start_range+num_skps):
      gsutil_args.append('%s/%s/*.skp' % (str(remote_dir), i))
    gsutil_args.append(str(slave_dest_dir))
    try:
      self.m.gsutil(gsutil_args, use_retry_wrapper=False)
    except self.m.step.StepFailure:
      # Some subdirectories might have no SKPs in them.
      pass
