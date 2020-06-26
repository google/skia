# Copyright 2020 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cat ./perfetto_config.pb | adb shell perfetto -c - -o /data/misc/perfetto-traces/trace.pb
adb pull /data/misc/perfetto-traces/trace.pb trace
