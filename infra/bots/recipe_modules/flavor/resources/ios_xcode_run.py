# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import json
import os
import plistlib
import subprocess
import sys


# Usage: ios_xcode_run.py <path to xcode> <path to app> <bundle id> <args>...
# Runs the given app via xcode with the given args.


xcode_path = sys.argv[1]
app_path = sys.argv[2]
bundle_id = sys.argv[3]
args = sys.argv[4:]

# Ensure xcrun uses the requested Xcode version.
os.environ['DEVELOPER_DIR'] = xcode_path

xcodebuild = os.path.join(
    xcode_path, 'Contents', 'Developer', 'usr', 'bin', 'xcodebuild')

subprocess.check_call([xcodebuild, '-version'])

# Find the ID of the attached device.
udid = subprocess.check_output(['idevice_id', '--list']).decode().strip().splitlines()[0]

# Try installing and launching via devicectl.
install_cmd = [
    'xcrun', 'devicectl', 'device', 'install', 'app',
    '--device', udid,
    app_path,
]
print('Installing app: %s' % ' '.join(install_cmd))
if subprocess.call(install_cmd) == 0:
  launch_cmd = [
      'xcrun', 'devicectl', 'device', 'process', 'launch',
      '--device', udid,
      '--terminate-existing',
      '--console',
      bundle_id,
  ] + args
  print('Launching app: %s' % ' '.join(launch_cmd))
  result = subprocess.call(launch_cmd)
  print('devicectl launch exited with code %d' % result)
  sys.exit(result)

# Fall back to xcodebuild for iOS < 17.
# TODO(borenet): Remove once all of our devices are updated.
print('devicectl failed; falling back to xcodebuild...')

# Write the .xctestrun file.
workdir = os.getcwd()
xctestrun_path = os.path.join(workdir, 'skia_tests.xctestrun')
module_name = os.path.splitext(os.path.basename(app_path))[0] + '_module'
contents = {
  module_name: {
    'TestBundlePath': app_path,
    'TestHostPath': app_path,
    'TestHostBundleIdentifier': bundle_id,
    'TestingEnvironmentVariables': {},
    'EnvironmentVariables': {},
    'CommandLineArguments': args,
  },
}
with open(xctestrun_path, 'wb') as f:
  plistlib.dump(contents, f)

destination = 'id=' + udid
output_json_path = os.path.join(workdir, 'enumerate-tests.json')

# Run the app via XCode.
result = subprocess.call([
  xcodebuild, 'test-without-building',
  '-xctestrun', xctestrun_path,
  '-destination', destination,
  '-enumerate-tests',
  '-test-enumeration-format', 'json',
  '-test-enumeration-output-path', output_json_path,
])

if os.path.exists(output_json_path):
  with open(output_json_path) as f:
    tests = json.load(f)
  print(tests)

sys.exit(result)
