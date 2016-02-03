#!/usr/bin/python

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import hashlib
import os
import platform
import stat
import subprocess
import sys
import urllib2

THIS_DIR = os.path.abspath(os.path.dirname(__file__))
MOJO_DIR = os.path.abspath(os.path.join(THIS_DIR, '../../third_party/externals/mojo'))

def GetFile(filename, bucket_directory):
    def sha1hash(path):
        hasher = hashlib.sha1()
        if os.path.isfile(path):
            with open(path, 'r') as f:
                hasher.update(f.read())
        return hasher.hexdigest()
    sha_path = filename + '.sha1'
    assert os.path.isfile(sha_path)
    with open(sha_path, 'r') as f:
        sha = f.read(40)
    if sha1hash(filename) == sha:
        return
    url = 'https://storage.googleapis.com/%s/%s' % (bucket_directory, sha)
    with open(filename, 'w') as o:
        o.write(urllib2.urlopen(url).read())
    os.chmod(filename, stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH |
                       stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
    assert sha1hash(filename) == sha

def GenerateBindings(path, cdir=None):
    system = (platform.machine(), platform.system())
    if ('x86_64', 'Darwin') == system:
        parser = 'public/tools/bindings/mojom_parser/bin/mac64/mojom_parser'
        bucket_directory = 'mojo/mojom_parser/mac64'
    elif ('x86_64', 'Linux') == system:
        parser = 'public/tools/bindings/mojom_parser/bin/linux64/mojom_parser'
        bucket_directory = 'mojo/mojom_parser/linux64'
    else:
        assert False
    GetFile(os.path.join(MOJO_DIR, parser), bucket_directory)
    assert os.path.isfile(path)
    path = os.path.abspath(path)
    exe = os.path.join(
        MOJO_DIR, 'public/tools/bindings/mojom_bindings_generator.py')
    assert os.path.isfile(exe)
    if cdir is None:
        cdir = os.path.dirname(path)
    assert os.path.isdir(cdir)
    cwd = os.getcwd()
    os.chdir(cdir)
    subprocess.check_call([exe, os.path.relpath(path, cdir)])
    os.chdir(cwd)

if __name__ == '__main__':
    if 1 == len(sys.argv):
        for f in [
            'public/interfaces/bindings/interface_control_messages.mojom',
            'public/interfaces/application/service_provider.mojom',
            'public/interfaces/bindings/tests/ping_service.mojom',
            'public/interfaces/application/application.mojom',
            ]:
            GenerateBindings(os.path.join(MOJO_DIR, f), os.path.join(MOJO_DIR, os.pardir))
    else:
        for arg in sys.argv[1:]:
            GenerateBindings(arg)
