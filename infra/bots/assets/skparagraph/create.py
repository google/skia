#!/usr/bin/env python3
#
# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""
The fonts collected by this script are used by SkParagraphTests.cpp which uses measurements
that are very particular to the specific font being used. Thus, we try to get the fonts from
a repeatable, documented source.
"""


import argparse
import os
import subprocess
import tempfile
import shutil

# NotoNaskhArabic-Regular.ttf from https://fonts.google.com/noto/specimen/Noto+Naskh+Arabic
# The fonts.google.com website seems to download the various .ttf files and then zip them client
# side. By using DevTools to watch what happens when the Download Family button is pressed, and
# then using sha256sum to verify the file in the .zip (with the nice name) matches the
# indecipherable url, one can find the following link. I mirrored this to
# https://storage.googleapis.com/skia-cdn/google-web-fonts/NotoNaskhArabic-Regular.ttf
# in case the gstatic links "expire" at some point.
# We cannot easily look at the .woff2 links from
# https://fonts.googleapis.com/css2?family=Noto%20Naskh%20Arabic
# as those seem to each have a subset of the unicode range and that makes our tests awkward.
ARABIC_URL = 'https://fonts.gstatic.com/s/notonaskharabic/v33/RrQ5bpV-9Dd1b1OAGA6M9PkyDuVBePeKNaxcsss0Y7bwvc5krK0z9_Mnuw.ttf'
ARABIC_SHA256 = 'b957e8c71a24e50c1aad4df775c46282bbe5e62e2b2b2ca72b153d75b6a15edd'

def create_asset(target_dir):
  """Copy the fonts from two different git repos into one folder."""
  os.makedirs(target_dir, exist_ok=True)
  with tempfile.TemporaryDirectory() as tmp:
    os.chdir(tmp)
    subprocess.call(['git', 'clone', 'https://github.com/Rusino/textlayout'])
    subprocess.call(['git', 'clone', 'https://skia.googlesource.com/skia/'])

    os.chdir(os.path.join(tmp, "textlayout"))
    subprocess.call(['git', 'checkout', '9c1868e84da1db358807ebff5cf52327e53560a0'])
    shutil.copytree("fonts", target_dir, dirs_exist_ok=True)

    os.chdir(os.path.join(tmp, "skia"))
    subprocess.call(['git', 'checkout', '2f82ef6e77774dc4e8e382b2fb6159c58c0f8725'])
    shutil.copytree(os.path.join("resources", "fonts"), target_dir, dirs_exist_ok=True)
    # Cleanup files that are not fonts needed for tests
    shutil.rmtree(os.path.join(target_dir, "abc"))
    shutil.rmtree(os.path.join(target_dir, "svg"))
    os.remove(os.path.join(target_dir, "fonts.xml"))

  target_file = os.path.join(target_dir, 'NotoNaskhArabic-Regular.ttf')
  subprocess.call(['wget', '--quiet', '--output-document', target_file, ARABIC_URL])
  output = subprocess.check_output(['sha256sum', target_file], encoding='utf-8')
  actual_hash = output.split(' ')[0]
  if actual_hash != ARABIC_SHA256:
    raise Exception('SHA256 does not match (%s != %s)' % (actual_hash, ARABIC_SHA256))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

