#!/usr/bin/env python2.7

import os
import shutil
import subprocess
import sys
import tempfile

# Arguments to the script:
#  app              path to binary to package, e.g. out/Debug/dm
#  identity         code signing identity, a long hex string
#                   (run security find-identity -v -p codesigning | grep Google)
#  mobileprovision  path to Google_Development.mobileprovision
#  prefix           ApplicationIdentifierPrefix, a short hex string in
#                   Google_Development.mobileprovision
app, identity, mobileprovision, prefix = sys.argv[1:]

out, app = os.path.split(app)

pkg = os.path.join(out, app + '.app')
if not os.path.exists(pkg):
  os.mkdir(pkg)

shutil.copy(os.path.join(out, app), pkg)
shutil.copy(mobileprovision,
            os.path.join(pkg, 'embedded.mobileprovision'))

with open(os.path.join(pkg, 'PkgInfo'), 'w') as f:
  f.write('APPL????')

with open(os.path.join(pkg, 'Info.plist'), 'w') as f:
  f.write('''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
                       "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>      <string>English</string>
  <key>CFBundleDisplayName</key>            <string>{app}</string>
  <key>CFBundleExecutable</key>             <string>{app}</string>
  <key>CFBundleIconFile</key>               <string></string>
  <key>CFBundleIdentifier</key>             <string>com.google.{app}</string>
  <key>CFBundleInfoDictionaryVersion</key>  <string>6.0</string>
  <key>CFBundleName</key>                   <string>{app}</string>
  <key>CFBundlePackageType</key>            <string>APPL</string>
  <key>CFBundleSignature</key>              <string>????</string>
  <key>CFBundleVersion</key>                <string>1.0</string>
  <key>LSRequiresIPhoneOS</key>             <true/>
  <key>NSMainNibFile</key>                  <string>MainWindow_iPhone</string>
  <key>NSMainNibFile~ipad</key>             <string>MainWindow_iPad</string>

  <key>UISupportedInterfaceOrientations</key>
  <array>
    <string>UIInterfaceOrientationPortrait</string>
  </array>

  <key>UISupportedInterfaceOrientations~ipad</key>
  <array>
    <string>UIInterfaceOrientationPortrait</string>
    <string>UIInterfaceOrientationPortraitUpsideDown</string>
    <string>UIInterfaceOrientationLandscapeLeft</string>
    <string>UIInterfaceOrientationLandscapeRight</string>
  </array>
</dict>
</plist>
'''.format(app=app))
subprocess.check_call(['plutil', '-convert', 'binary1',
                       os.path.join(pkg, 'Info.plist')])

subprocess.check_call([
  'ibtool', '--compile', os.path.join(pkg, 'MainWindow_iPhone.nib'),
  '/Users/mtklein/skia/experimental/iOSSampleApp/iPhone/MainWindow_iPhone.xib'])

subprocess.check_call([
  'ibtool', '--compile', os.path.join(pkg, 'MainWindow_iPad.nib'),
  '/Users/mtklein/skia/experimental/iOSSampleApp/iPad/MainWindow_iPad.xib'])

with tempfile.NamedTemporaryFile(suffix='.xcent') as f:
  f.write('''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
                       "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>application-identifier</key>  <string>{prefix}.com.google.{app}</string>
  <key>get-task-allow</key>          <true/>
  <key>keychain-access-groups</key>
  <array>
    <string>{prefix}.com.google.{app}</string>
  </array>
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
