#!/usr/bin/env python
#
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the mesa driver. This defers to a Docker container
   with the build_mesa.sh script."""


import argparse
import subprocess

# TODO(dogben): In the future, it might be simpler to build the docker image as
# part of this script so that we don't need to push it to the container repo.
# Doing so would make this script more repeatable, since someone could
# accidentally change the Docker image that "v2" points to.
DOCKER_IMAGE = 'gcr.io/skia-public/mesa-driver-builder:v2'
BUILD_SCRIPT = '/opt/build_mesa.sh'
MESA_VERSION = '18.3.3'


def create_asset(target_dir):
  """Create the asset."""
  cmd = [
    'docker', 'run', '--volume', '%s:/OUT' % target_dir,
    '--env', 'MESA_VERSION=%s' % MESA_VERSION,
    DOCKER_IMAGE, BUILD_SCRIPT
  ]
  print('Running docker cmd', cmd)
  subprocess.check_output(cmd)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
