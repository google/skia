#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Upload DM output PNG files and JSON summary to Google Storage."""


import datetime
import os
import shutil
import sys
import tempfile

def main(dm_dir, build_number, builder_name):
  """Upload DM output PNG files and JSON summary to Google Storage.

    dm_dir:        path to PNG files and JSON summary    (str)
    build_number:  nth build on this builder             (str or int)
    builder_name:  name of this builder                  (str)
  """
  # import gs_utils
  current_dir = os.path.dirname(os.path.abspath(__file__))
  sys.path.insert(0, os.path.join(current_dir, "../../../common/py/utils"))
  import gs_utils

  # Private, but Google-readable.
  ACL = gs_utils.GSUtils.PredefinedACL.PRIVATE
  FINE_ACLS = [(
    gs_utils.GSUtils.IdType.GROUP_BY_DOMAIN,
    'google.com',
    gs_utils.GSUtils.Permission.READ
  )]

  if not os.path.isfile(os.path.join(dm_dir, 'dm.json')):
    sys.exit("no dm.json file found in output directory.")

  # Move dm.json to its own directory to make uploading it easier.
  tmp = tempfile.mkdtemp()
  shutil.move(os.path.join(dm_dir, 'dm.json'),
              os.path.join(tmp,    'dm.json'))

  # Only images are left in dm_dir.  Upload any new ones.
  gs = gs_utils.GSUtils()
  gs.upload_dir_contents(dm_dir,
                         'skia-android-dm',
                         'dm-images-v1',
                         upload_if = gs.UploadIf.IF_NEW,
                         predefined_acl = ACL,
                         fine_grained_acl_list = FINE_ACLS)


  # /dm-json-v1/year/month/day/hour/build-number/builder/dm.json
  now = datetime.datetime.utcnow()
  summary_dest_dir = '/'.join(['dm-json-v1',
                               str(now.year ).zfill(4),
                               str(now.month).zfill(2),
                               str(now.day  ).zfill(2),
                               str(now.hour ).zfill(2),
                               str(build_number),
                               builder_name])

  # Upload the JSON summary.
  gs.upload_dir_contents(tmp,
                         'skia-android-dm',
                         summary_dest_dir,
                         predefined_acl = ACL,
                         fine_grained_acl_list = FINE_ACLS)


  # Just for hygiene, put dm.json back.
  shutil.move(os.path.join(tmp,    'dm.json'),
              os.path.join(dm_dir, 'dm.json'))
  os.rmdir(tmp)

if '__main__' == __name__:
  main(*sys.argv[1:])
