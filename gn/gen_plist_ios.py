#!/usr/bin/env python2.7
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

# Arguments to the script:
#  app              path to binary to package, e.g. out/Debug/gen/dm
app, = sys.argv[1:]

# Write a minimal Info.plist to name the package and point at the binary.
out_directory = app + '.d'
app = os.path.basename(app)
if not os.path.exists(out_directory):
  os.mkdir(out_directory)
with open(os.path.join(out_directory, 'Info.plist'), 'w') as f:
  f.write('''
<plist version="1.0">
  <dict>
    <key>CFBundleVersion</key> <string>0.1.0</string>
    <key>CFBundleShortVersionString</key> <string>0.1.0</string>
    <key>CFBundleExecutable</key> <string>{app}</string>
    <key>CFBundleIdentifier</key> <string>com.google.{app}</string>
    <key>CFBundlePackageType</key> <string>APPL</string>
    <key>LSRequiresIPhoneOS</key> <true/>
    <key>UIDeviceFamily</key> <array>
      <integer>1</integer>
      <integer>2</integer>
    </array>
    <key>UILaunchStoryboardName</key> <string>LaunchScreen</string>
  </dict>
</plist>
'''.format(app=app))
