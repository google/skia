# Copyright 2018 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess

out_file = open("asset_resources.gni", "w")

files = subprocess.check_output(["find", "../platform_tools/android/apps/skqp/src/main/assets", "-type", "f"])
file_lines = files.splitlines()
out_file.write('asset_resources = [\n')

for file in file_lines:
	file_split = file.split('/')
	file_split = file_split[8:]
	out_file.write('  {\n')
	out_file.write('    path=rebase_path("%s")\n' % file)
	out_file.write('    dest="%s"\n' % '/'.join(file_split))
	out_file.write('  },\n')

out_file.write(']\n')
