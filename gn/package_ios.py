#!/usr/bin/env python2.7
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import glob
import os
import re
import shutil
import subprocess
import sys
import tempfile

# Arguments to the script:
#  app              path to binary to package, e.g. out/Debug/dm
app, = sys.argv[1:]

# Find the Google signing identity.
identity = None
for line in subprocess.check_output(['security', 'find-identity']).split('\n'):
  m = re.match(r'''.*\) (.*) ".*Google.*"''', line)
  if m:
    identity = m.group(1)
assert identity

# Find the Google mobile provisioning profile.
mobileprovision = None
for p in glob.glob(os.path.join(os.environ['HOME'], 'Library', 'MobileDevice',
                                'Provisioning Profiles', '*.mobileprovision')):
  if re.search(r'''<key>Name</key>
\t<string>Google Development</string>''', open(p).read(), re.MULTILINE):
    mobileprovision = p
assert mobileprovision

out, app = os.path.split(app)

pkg = os.path.join(out, app + '.app')
if not os.path.exists(pkg):
  os.mkdir(pkg)

# The binary and .mobileprovision just get copied into the package.
shutil.copy(os.path.join(out, app), pkg)
shutil.copy(mobileprovision,
            os.path.join(pkg, 'embedded.mobileprovision'))

# Write a minimal Info.plist to name the package and point at the binary.
with open(os.path.join(pkg, 'Info.plist'), 'w') as f:
  f.write('''
<plist version="1.0">
  <dict>
    <key>CFBundleExecutable</key> <string>{app}</string>
    <key>CFBundleIdentifier</key> <string>com.google.{app}</string>
  </dict>
</plist>
'''.format(app=app))

# Extract the appliciation identitifer prefix from the .mobileprovision.
m = re.search(r'''<key>ApplicationIdentifierPrefix</key>
\t<array>
\t<string>(.*)</string>''', open(mobileprovision).read(), re.MULTILINE)
prefix = m.group(1)

# Write a minimal entitlements file, then codesign.
with tempfile.NamedTemporaryFile() as f:
  f.write('''
<plist version="1.0">
  <dict>
    <key>application-identifier</key> <string>{prefix}.com.google.{app}</string>
    <key>get-task-allow</key>         <true/>
  </dict>
</plist>
'''.format(prefix=prefix, app=app))
  f.flush()

  subprocess.check_call(['codesign',
                         '--force',
                         '--sign', identity,
                         '--entitlements', f.name,
                         '--timestamp=none',
                         pkg])
