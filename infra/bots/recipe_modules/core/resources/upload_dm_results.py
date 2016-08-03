#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Upload DM output PNG files and JSON summary to Google Storage."""

import datetime
import json
import os
import shutil
import sys
import tempfile

def main(dm_dir, git_hash, builder_name, build_number, try_issue, import_path):
  """Upload DM output PNG files and JSON summary to Google Storage.

    dm_dir:        path to PNG files and JSON summary    (str)
    git_hash:      this build's Git hash                 (str)
    builder_name:  name of this builder                  (str)
    build_number:  nth build on this builder             (str or int)
    try_issue:     Rietveld issue if this is a try job   (str, int, or None)
    import_path:   Path to import the gs_utils package   (str)
  """
  # import gs_utils
  sys.path.insert(0, import_path)
  import gs_utils

  # Private, but Google-readable.
  ACL = gs_utils.GSUtils.PredefinedACL.PRIVATE
  FINE_ACLS = [(
    gs_utils.GSUtils.IdType.GROUP_BY_DOMAIN,
    'google.com',
    gs_utils.GSUtils.Permission.READ
  )]

  # Move dm.json and verbose.log to their own directory for easy upload.
  tmp = tempfile.mkdtemp()
  shutil.move(os.path.join(dm_dir, 'dm.json'),
              os.path.join(tmp,    'dm.json'))
  shutil.move(os.path.join(dm_dir, 'verbose.log'),
              os.path.join(tmp,    'verbose.log'))

  # Make sure the JSON file parses correctly.
  json_file_name = os.path.join(tmp, 'dm.json')
  with open(json_file_name) as jsonFile:
    try:
      json.load(jsonFile)
    except ValueError:
      json_content = open(json_file_name).read()
      print >> sys.stderr, "Invalid JSON: \n\n%s\n" % json_content
      raise

  # Only images are left in dm_dir.  Upload any new ones.
  gs = gs_utils.GSUtils()
  bucket, image_dest_dir = 'chromium-skia-gm', 'dm-images-v1'
  print 'Uploading images to gs://' + bucket + '/' + image_dest_dir
  gs.upload_dir_contents(dm_dir,
                         bucket,
                         image_dest_dir,
                         upload_if = gs.UploadIf.ALWAYS,
                         predefined_acl = ACL,
                         fine_grained_acl_list = FINE_ACLS)


  # /dm-json-v1/year/month/day/hour/git-hash/builder/build-number/dm.json
  now = datetime.datetime.utcnow()
  summary_dest_dir = '/'.join(['dm-json-v1',
                               str(now.year ).zfill(4),
                               str(now.month).zfill(2),
                               str(now.day  ).zfill(2),
                               str(now.hour ).zfill(2),
                               git_hash,
                               builder_name,
                               str(build_number)])

  # Trybot results are further siloed by CL.
  if try_issue:
    summary_dest_dir = '/'.join(['trybot', summary_dest_dir, str(try_issue)])

  # Upload the JSON summary and verbose.log.
  print 'Uploading logs to gs://' + bucket + '/' + summary_dest_dir
  gs.upload_dir_contents(tmp,
                         bucket,
                         summary_dest_dir,
                         predefined_acl = ACL,
                         fine_grained_acl_list = FINE_ACLS)


  # Just for hygiene, put dm.json and verbose.log back.
  shutil.move(os.path.join(tmp,    'dm.json'),
              os.path.join(dm_dir, 'dm.json'))
  shutil.move(os.path.join(tmp,    'verbose.log'),
              os.path.join(dm_dir, 'verbose.log'))
  os.rmdir(tmp)

if '__main__' == __name__:
  main(*sys.argv[1:])
