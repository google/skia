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
#  bundle_prefix    the first part of the bundle ID, e.g. com.google (no trailing '.')
#                   the app name will be appended to this to create the full bundle ID
app,bundle_prefix = sys.argv[1:3]

out, app = os.path.split(app)

# Write a minimal Info.plist to name the package and point at the binary.
with open(os.path.join(out, app + '_Info.plist'), 'w') as f:
  f.write('''
<plist version="1.0">
  <dict>
    <key>CFBundleVersion</key> <string>0.1.0</string>
    <key>CFBundleShortVersionString</key> <string>0.1.0</string>
    <key>CFBundleExecutable</key> <string>{app}</string>
    <key>CFBundleIdentifier</key> <string>{bundle_prefix}.{app}</string>
    <key>CFBundlePackageType</key> <string>APPL</string>
    <key>LSRequiresIPhoneOS</key> <true/>
    <key>UIDeviceFamily</key> <array>
      <integer>1</integer>
      <integer>2</integer>
    </array>
    <key>UILaunchStoryboardName</key> <string>LaunchScreen</string>
  </dict>
</plist>
'''.format(app=app, bundle_prefix=bundle_prefix))
