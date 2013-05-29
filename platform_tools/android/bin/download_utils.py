#!/usr/bin/python
# Copyright (c) 2012 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A library to assist automatically downloading files.

This library is used by scripts that download tarballs, zipfiles, etc. as part
of the build process.
"""

import hashlib
import http_download
import os.path
import re
import shutil
import sys
import time
import urllib2

SOURCE_STAMP = 'SOURCE_URL'
HASH_STAMP = 'SOURCE_SHA1'


# Designed to handle more general inputs than sys.platform because the platform
# name may come from the command line.
PLATFORM_COLLAPSE = {
    'windows': 'windows',
    'win32': 'windows',
    'cygwin': 'windows',
    'linux': 'linux',
    'linux2': 'linux',
    'linux3': 'linux',
    'darwin': 'mac',
    'mac': 'mac',
}

ARCH_COLLAPSE = {
    'i386'  : 'x86',
    'i686'  : 'x86',
    'x86_64': 'x86',
    'armv7l': 'arm',
}


class HashError(Exception):
  def __init__(self, download_url, expected_hash, actual_hash):
    self.download_url = download_url
    self.expected_hash = expected_hash
    self.actual_hash = actual_hash

  def __str__(self):
    return 'Got hash "%s" but expected hash "%s" for "%s"' % (
        self.actual_hash, self.expected_hash, self.download_url)


def PlatformName(name=None):
  if name is None:
    name = sys.platform
  return PLATFORM_COLLAPSE[name]

def ArchName(name=None):
  if name is None:
    if PlatformName() == 'windows':
      # TODO(pdox): Figure out how to auto-detect 32-bit vs 64-bit Windows.
      name = 'i386'
    else:
      import platform
      name = platform.machine()
  return ARCH_COLLAPSE[name]

def EnsureFileCanBeWritten(filename):
  directory = os.path.dirname(filename)
  if not os.path.exists(directory):
    os.makedirs(directory)


def WriteData(filename, data):
  EnsureFileCanBeWritten(filename)
  f = open(filename, 'wb')
  f.write(data)
  f.close()


def WriteDataFromStream(filename, stream, chunk_size, verbose=True):
  EnsureFileCanBeWritten(filename)
  dst = open(filename, 'wb')
  try:
    while True:
      data = stream.read(chunk_size)
      if len(data) == 0:
        break
      dst.write(data)
      if verbose:
        # Indicate that we're still writing.
        sys.stdout.write('.')
        sys.stdout.flush()
  finally:
    if verbose:
      sys.stdout.write('\n')
    dst.close()


def DoesStampMatch(stampfile, expected, index):
  try:
    f = open(stampfile, 'r')
    stamp = f.read()
    f.close()
    if stamp.split('\n')[index] == expected:
      return "already up-to-date."
    elif stamp.startswith('manual'):
      return "manual override."
    return False
  except IOError:
    return False


def WriteStamp(stampfile, data):
  EnsureFileCanBeWritten(stampfile)
  f = open(stampfile, 'w')
  f.write(data)
  f.close()


def StampIsCurrent(path, stamp_name, stamp_contents, min_time=None, index=0):
  stampfile = os.path.join(path, stamp_name)

  # Check if the stampfile is older than the minimum last mod time
  if min_time:
    try:
      stamp_time = os.stat(stampfile).st_mtime
      if stamp_time <= min_time:
        return False
    except OSError:
      return False

  return DoesStampMatch(stampfile, stamp_contents, index)


def WriteSourceStamp(path, url):
  stampfile = os.path.join(path, SOURCE_STAMP)
  WriteStamp(stampfile, url)

def WriteHashStamp(path, hash_val):
  hash_stampfile = os.path.join(path, HASH_STAMP)
  WriteStamp(hash_stampfile, hash_val)


def Retry(op, *args):
  # Windows seems to be prone to having commands that delete files or
  # directories fail.  We currently do not have a complete understanding why,
  # and as a workaround we simply retry the command a few times.
  # It appears that file locks are hanging around longer than they should.  This
  # may be a secondary effect of processes hanging around longer than they
  # should.  This may be because when we kill a browser sel_ldr does not exit
  # immediately, etc.
  # Virus checkers can also accidently prevent files from being deleted, but
  # that shouldn't be a problem on the bots.
  if sys.platform in ('win32', 'cygwin'):
    count = 0
    while True:
      try:
        op(*args)
        break
      except Exception:
        sys.stdout.write("FAILED: %s %s\n" % (op.__name__, repr(args)))
        count += 1
        if count < 5:
          sys.stdout.write("RETRY: %s %s\n" % (op.__name__, repr(args)))
          time.sleep(pow(2, count))
        else:
          # Don't mask the exception.
          raise
  else:
    op(*args)


def MoveDirCleanly(src, dst):
  RemoveDir(dst)
  MoveDir(src, dst)


def MoveDir(src, dst):
  Retry(shutil.move, src, dst)


def RemoveDir(path):
  if os.path.exists(path):
    Retry(shutil.rmtree, path)


def RemoveFile(path):
  if os.path.exists(path):
    Retry(os.unlink, path)


def _HashFileHandle(fh):
  """sha1 of a file like object.

  Arguments:
    fh: file handle like object to hash.
  Returns:
    sha1 as a string.
  """
  hasher = hashlib.sha1()
  try:
    while True:
      data = fh.read(4096)
      if not data:
        break
      hasher.update(data)
  finally:
    fh.close()
  return hasher.hexdigest()


def HashFile(filename):
  """sha1 a file on disk.

  Arguments:
    filename: filename to hash.
  Returns:
    sha1 as a string.
  """
  fh = open(filename, 'rb')
  return _HashFileHandle(fh)


def HashUrlByDownloading(url):
  """sha1 the data at an url.

  Arguments:
    url: url to download from.
  Returns:
    sha1 of the data at the url.
  """
  try:
    fh = urllib2.urlopen(url)
  except:
    sys.stderr.write("Failed fetching URL: %s\n" % url)
    raise
  return _HashFileHandle(fh)


# Attempts to get the SHA1 hash of a file given a URL by looking for
# an adjacent file with a ".sha1hash" suffix.  This saves having to
# download a large tarball just to get its hash.  Otherwise, we fall
# back to downloading the main file.
def HashUrl(url):
  hash_url = '%s.sha1hash' % url
  try:
    fh = urllib2.urlopen(hash_url)
    data = fh.read(100)
    fh.close()
  except urllib2.HTTPError, exn:
    if exn.code == 404:
      return HashUrlByDownloading(url)
    raise
  else:
    if not re.match('[0-9a-f]{40}\n?$', data):
      raise AssertionError('Bad SHA1 hash file: %r' % data)
    return data.strip()


def SyncURL(url, filename=None, stamp_dir=None, min_time=None,
            hash_val=None, keep=False, verbose=False, stamp_index=0):
  """Synchronize a destination file with a URL

  if the URL does not match the URL stamp, then we must re-download it.

  Arugments:
    url: the url which will to compare against and download
    filename: the file to create on download
    path: the download path
    stamp_dir: the filename containing the URL stamp to check against
    hash_val: if set, the expected hash which must be matched
    verbose: prints out status as it runs
    stamp_index: index within the stamp file to check.
  Returns:
    True if the file is replaced
    False if the file is not replaced
  Exception:
    HashError: if the hash does not match
  """

  assert url and filename

  # If we are not keeping the tarball, or we already have it, we can
  # skip downloading it for this reason. If we are keeping it,
  # it must exist.
  if keep:
    tarball_ok = os.path.isfile(filename)
  else:
    tarball_ok = True

  # If we don't need the tarball and the stamp_file matches the url, then
  # we must be up to date.  If the URL differs but the recorded hash matches
  # the one we'll insist the tarball has, then that's good enough too.
  # TODO(mcgrathr): Download the .sha1sum file first to compare with
  # the cached hash, in case --file-hash options weren't used.
  if tarball_ok and stamp_dir is not None:
    if StampIsCurrent(stamp_dir, SOURCE_STAMP, url, min_time):
      if verbose:
        print '%s is already up to date.' % filename
      return False
    if (hash_val is not None and
        StampIsCurrent(stamp_dir, HASH_STAMP, hash_val, min_time, stamp_index)):
      if verbose:
        print '%s is identical to the up to date file.' % filename
      return False

  if verbose:
    print 'Updating %s\n\tfrom %s.' % (filename, url)
  EnsureFileCanBeWritten(filename)
  http_download.HttpDownload(url, filename)

  if hash_val:
    tar_hash = HashFile(filename)
    if hash_val != tar_hash:
      raise HashError(actual_hash=tar_hash, expected_hash=hash_val,
                      download_url=url)

  return True
