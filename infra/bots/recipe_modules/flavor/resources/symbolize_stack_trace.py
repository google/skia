#!/usr/bin/env python3
# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import collections
import os
import re
import subprocess
import sys


# Run a command and symbolize anything that looks like a stacktrace in the
# stdout/stderr. This will return with the same error code as the command.
def main(basedir, cmd):
    logs = collections.deque(maxlen=500)

    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT,
                            encoding='ISO-8859-1')
    for line in iter(proc.stdout.readline, ''):
        sys.stdout.write(line)
        logs.append(line)
    proc.wait()
    print('Command exited with code %s' % proc.returncode)

    try:
        subprocess.check_call(['addr2line', '--help'],
                        stdout=subprocess.DEVNULL,
                        stderr=subprocess.DEVNULL)
    except OSError:
        print('addr2line not found on PATH. Skipping symbolization.')
        return


    # Stacktraces generally look like:
    # /lib/x86_64-linux-gnu/libc.so.6(abort+0x16a) [0x7fa90e8d0c62]
    # /b/s/w/irISUIyA/linux_vulkan_intel_driver_debug/./libvulkan_intel.so(+0x1f4d0a) [0x7fa909eead0a]
    # /b/s/w/irISUIyA/out/Debug/dm() [0x17c3c5f]
    # The stack_line regex splits those into three parts. Experimentation has
    # shown that the address in () works best for external libraries, but our code
    # doesn't have that. So, we capture both addresses and prefer using the first
    # over the second, unless the first is blank or invalid. Relative offsets
    # like abort+0x16a are ignored.
    stack_line = r'^(?P<path>.+)\(\+?(?P<addr>.*)\) ?\[(?P<addr2>.+)\]'
    # After performing addr2line, the result can be something obnoxious like:
    # foo(bar) at /b/s/w/a39kd/Skia/out/Clang/../../src/gpu/Frobulator.cpp:13
    # The extra_path strips off the not-useful prefix and leaves just the
    # important src/gpu/Frobulator.cpp:13 bit.
    extra_path = r'/.*\.\./'
    is_first = True
    last = ""
    for line in logs:
        line = line.strip()
        # For unknown reasons, sometimes lines are duplicated in the stacktrace.
        # This removes the duplication
        if line == last:
            continue
        last = line

        m = re.search(stack_line, line)
        if m:
            if is_first:
                print('#######################################')
                print('symbolized stacktrace follows')
                print('#######################################')
                is_first = False

            path = m.group('path')
            addr = m.group('addr')
            addr2 = m.group('addr2')
            if os.path.exists(path):
                if not addr or not addr.startswith('0x'):
                    addr = addr2
                try:
                    sym = subprocess.check_output([
                        'addr2line', '--demangle', '--pretty-print', '--functions',
                        '--exe='+path, addr
                    ]).decode('utf-8')
                except subprocess.CalledProcessError:
                    sym = ''
                sym = sym.strip()
                # If addr2line doesn't return anything useful, we don't replace the
                # original address, so the human can see it.
                if sym and not sym.startswith('?'):
                    if path.startswith(basedir):
                        path = path[len(basedir) + 1:]
                    sym = re.sub(extra_path, '', sym)
                    line = path + ' ' + sym
            print(line)

    sys.exit(proc.returncode)


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('USAGE: %s working_dir cmd_and_args...' % sys.argv[0],
              file=sys.stderr)
        sys.exit(1)
    main(sys.argv[1], sys.argv[2:])
