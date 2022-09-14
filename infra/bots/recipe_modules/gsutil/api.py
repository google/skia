# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api

UPLOAD_ATTEMPTS = 5

class GSUtilApi(recipe_api.RecipeApi):
  def __call__(self, step_name, *args):
    """Run gsutil with the given args.

       This assumes there exists an executable called gsutil on the PATH.
       This probably only works for Linux/Mac, but those are the only
       hosts that we try to upload to GCS from anyway.
    """
    return self.m.step(step_name, cmd=['gsutil'] + list(args))

  def cp(self, name, src, dst, extra_gsutil_args=None, extra_args=None,
         multithread=False):
    """Attempt to upload or download files to/from Google Cloud Storage (GCS).

    Args:
      name: string. Will be used to fill out the step name.
      src: string. Absolute path for a local file or gcs file (e.g. gs://...)
      dst: string. Same as src.
      extra_gsutil_args: optional list of args to be passed to gsutil before the
        cp command.
      extra_args: optional list of args to be passed to gsutil cp. e.g. [-Z]
        asks all files be compressed with gzip after upload and before download.
      multi_thread: if the -m argument should be used to copy multiple items
        at once (e.g. gsutil -m cp foo* gs://bar/dir)

    If the operation fails, it will be retried multiple times.
    """
    cmd = []
    if extra_gsutil_args:
      cmd.extend(extra_gsutil_args)
    if multithread:
      cmd.append('-m')
    cmd.append('cp')
    if extra_args:
      cmd.extend(extra_args)
    cmd.extend([src, dst])

    name = 'upload %s' % name
    for i in range(UPLOAD_ATTEMPTS):
      step_name = name
      if i > 0:
        step_name += ' (attempt %d)' % (i+1)
      try:
        self(step_name, *cmd)
        break
      except self.m.step.StepFailure:
        if i == UPLOAD_ATTEMPTS - 1:
          raise
