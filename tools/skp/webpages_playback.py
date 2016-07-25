#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Archives or replays webpages and creates SKPs in a Google Storage location.

To archive webpages and store SKP files (archives should be rarely updated):

cd skia
python tools/skp/webpages_playback.py --data_store=gs://rmistry --record \
--page_sets=all --skia_tools=/home/default/trunk/out/Debug/ \
--browser_executable=/tmp/chromium/out/Release/chrome

The above command uses Google Storage bucket 'rmistry' to download needed files.

To replay archived webpages and re-generate SKP files (should be run whenever
SkPicture.PICTURE_VERSION changes):

cd skia
python tools/skp/webpages_playback.py --data_store=gs://rmistry \
--page_sets=all --skia_tools=/home/default/trunk/out/Debug/ \
--browser_executable=/tmp/chromium/out/Release/chrome


Specify the --page_sets flag (default value is 'all') to pick a list of which
webpages should be archived and/or replayed. Eg:

--page_sets=tools/skp/page_sets/skia_yahooanswers_desktop.py,\
tools/skp/page_sets/skia_googlecalendar_nexus10.py

The --browser_executable flag should point to the browser binary you want to use
to capture archives and/or capture SKP files. Majority of the time it should be
a newly built chrome binary.

The --data_store flag controls where the needed artifacts, such as
credential files, are downloaded from. It also controls where the
generated artifacts, such as recorded webpages and resulting skp renderings,
are uploaded to. URLs with scheme 'gs://' use Google Storage. Otherwise
use local filesystem.

The --upload=True flag means generated artifacts will be
uploaded or copied to the location specified by --data_store. (default value is
False if not specified).

The --non-interactive flag controls whether the script will prompt the user
(default value is False if not specified).

The --skia_tools flag if specified will allow this script to run
debugger, render_pictures, and render_pdfs on the captured
SKP(s). The tools are run after all SKPs are succesfully captured to make sure
they can be added to the buildbots with no breakages.
"""

import glob
import optparse
import os
import posixpath
import shutil
import subprocess
import sys
import tempfile
import time
import traceback

SKIA_DIR = os.path.abspath(os.path.join(
    os.path.realpath(os.path.dirname(__file__)),
    os.pardir, os.pardir))
sys.path.insert(0, SKIA_DIR)

from common.py.utils import gs_utils
from common.py.utils import shell_utils

ROOT_PLAYBACK_DIR_NAME = 'playback'
SKPICTURES_DIR_NAME = 'skps'

PARTNERS_GS_BUCKET = 'gs://chrome-partner-telemetry'

# Local archive and SKP directories.
LOCAL_PLAYBACK_ROOT_DIR = os.path.join(
    tempfile.gettempdir(), ROOT_PLAYBACK_DIR_NAME)
LOCAL_REPLAY_WEBPAGES_ARCHIVE_DIR = os.path.join(
    os.path.abspath(os.path.dirname(__file__)), 'page_sets', 'data')
TMP_SKP_DIR = tempfile.mkdtemp()

# Location of the credentials.json file and the string that represents missing
# passwords.
CREDENTIALS_FILE_PATH = os.path.join(
    os.path.abspath(os.path.dirname(__file__)), 'page_sets', 'data',
    'credentials.json'
)

# Name of the SKP benchmark
SKP_BENCHMARK = 'skpicture_printer'

# The max base name length of Skp files.
MAX_SKP_BASE_NAME_LEN = 31

# Dictionary of device to platform prefixes for SKP files.
DEVICE_TO_PLATFORM_PREFIX = {
    'desktop': 'desk',
    'galaxynexus': 'mobi',
    'nexus10': 'tabl'
}

# How many times the record_wpr binary should be retried.
RETRY_RECORD_WPR_COUNT = 5
# How many times the run_benchmark binary should be retried.
RETRY_RUN_MEASUREMENT_COUNT = 5

# Location of the credentials.json file in Google Storage.
CREDENTIALS_GS_PATH = '/playback/credentials/credentials.json'

X11_DISPLAY = os.getenv('DISPLAY', ':0')

GS_PREDEFINED_ACL = gs_utils.GSUtils.PredefinedACL.PRIVATE
GS_FINE_GRAINED_ACL_LIST = [
  (gs_utils.GSUtils.IdType.GROUP_BY_DOMAIN, 'google.com',
   gs_utils.GSUtils.Permission.READ),
]

# Path to Chromium's page sets.
CHROMIUM_PAGE_SETS_PATH = os.path.join('tools', 'perf', 'page_sets')

# Dictionary of supported Chromium page sets to their file prefixes.
CHROMIUM_PAGE_SETS_TO_PREFIX = {
    'key_mobile_sites_smooth.py': 'keymobi',
    'top_25_smooth.py': 'top25desk',
}


def remove_prefix(s, prefix):
  if s.startswith(prefix):
    return s[len(prefix):]
  return s


class SkPicturePlayback(object):
  """Class that archives or replays webpages and creates SKPs."""

  def __init__(self, parse_options):
    """Constructs a SkPicturePlayback BuildStep instance."""
    assert parse_options.browser_executable, 'Must specify --browser_executable'
    self._browser_executable = parse_options.browser_executable
    self._browser_args = '--disable-setuid-sandbox'
    if parse_options.browser_extra_args:
      self._browser_args = '%s %s' % (
          self._browser_args, parse_options.browser_extra_args)

    self._chrome_page_sets_path = os.path.join(parse_options.chrome_src_path,
                                               CHROMIUM_PAGE_SETS_PATH)
    self._all_page_sets_specified = parse_options.page_sets == 'all'
    self._page_sets = self._ParsePageSets(parse_options.page_sets)

    self._record = parse_options.record
    self._skia_tools = parse_options.skia_tools
    self._non_interactive = parse_options.non_interactive
    self._upload = parse_options.upload
    self._skp_prefix = parse_options.skp_prefix
    data_store_location = parse_options.data_store
    if data_store_location.startswith(gs_utils.GS_PREFIX):
      self.gs = GoogleStorageDataStore(data_store_location)
    else:
      self.gs = LocalFileSystemDataStore(data_store_location)
    self._upload_to_partner_bucket = parse_options.upload_to_partner_bucket
    self._alternate_upload_dir = parse_options.alternate_upload_dir
    self._telemetry_binaries_dir = os.path.join(parse_options.chrome_src_path,
                                                'tools', 'perf')
    self._catapult_dir = os.path.join(parse_options.chrome_src_path,
                                      'third_party', 'catapult')

    self._local_skp_dir = os.path.join(
        parse_options.output_dir, ROOT_PLAYBACK_DIR_NAME, SKPICTURES_DIR_NAME)
    self._local_record_webpages_archive_dir = os.path.join(
        parse_options.output_dir, ROOT_PLAYBACK_DIR_NAME, 'webpages_archive')

    # List of SKP files generated by this script.
    self._skp_files = []

  def _ParsePageSets(self, page_sets):
    if not page_sets:
      raise ValueError('Must specify at least one page_set!')
    elif self._all_page_sets_specified:
      # Get everything from the page_sets directory.
      page_sets_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)),
                                   'page_sets')
      ps = [os.path.join(page_sets_dir, page_set)
            for page_set in os.listdir(page_sets_dir)
            if not os.path.isdir(os.path.join(page_sets_dir, page_set)) and
               page_set.endswith('.py')]
      chromium_ps = [
          os.path.join(self._chrome_page_sets_path, cr_page_set)
          for cr_page_set in CHROMIUM_PAGE_SETS_TO_PREFIX]
      ps.extend(chromium_ps)
    elif '*' in page_sets:
      # Explode and return the glob.
      ps = glob.glob(page_sets)
    else:
      ps = page_sets.split(',')
    ps.sort()
    return ps

  def _IsChromiumPageSet(self, page_set):
    """Returns true if the specified page set is a Chromium page set."""
    return page_set.startswith(self._chrome_page_sets_path)

  def Run(self):
    """Run the SkPicturePlayback BuildStep."""

    # Download the credentials file if it was not previously downloaded.
    if not os.path.isfile(CREDENTIALS_FILE_PATH):
      # Download the credentials.json file from Google Storage.
      self.gs.download_file(CREDENTIALS_GS_PATH, CREDENTIALS_FILE_PATH)

    if not os.path.isfile(CREDENTIALS_FILE_PATH):
      print """\n\nCould not locate credentials file in the storage.
      Please create a %s file that contains:
      {
        "google": {
          "username": "google_testing_account_username",
          "password": "google_testing_account_password"
        },
        "facebook": {
          "username": "facebook_testing_account_username",
          "password": "facebook_testing_account_password"
        }
      }\n\n""" % CREDENTIALS_FILE_PATH
      raw_input("Please press a key when you are ready to proceed...")

    # Delete any left over data files in the data directory.
    for archive_file in glob.glob(
        os.path.join(LOCAL_REPLAY_WEBPAGES_ARCHIVE_DIR, 'skia_*')):
      os.remove(archive_file)

    # Delete the local root directory if it already exists.
    if os.path.exists(LOCAL_PLAYBACK_ROOT_DIR):
      shutil.rmtree(LOCAL_PLAYBACK_ROOT_DIR)

    # Create the required local storage directories.
    self._CreateLocalStorageDirs()

    # Start the timer.
    start_time = time.time()

    # Loop through all page_sets.
    for page_set in self._page_sets:

      page_set_basename = os.path.basename(page_set).split('.')[0]
      page_set_json_name = page_set_basename + '.json'
      wpr_data_file = page_set.split(os.path.sep)[-1].split('.')[0] + '_000.wpr'
      page_set_dir = os.path.dirname(page_set)

      if self._IsChromiumPageSet(page_set):
        print 'Using Chromium\'s captured archives for Chromium\'s page sets.'
      elif self._record:
        # Create an archive of the specified webpages if '--record=True' is
        # specified.
        record_wpr_cmd = (
          'PYTHONPATH=%s:%s:$PYTHONPATH' % (page_set_dir, self._catapult_dir),
          'DISPLAY=%s' % X11_DISPLAY,
          os.path.join(self._telemetry_binaries_dir, 'record_wpr'),
          '--extra-browser-args="%s"' % self._browser_args,
          '--browser=exact',
          '--browser-executable=%s' % self._browser_executable,
          '%s_page_set' % page_set_basename,
          '--page-set-base-dir=%s' % page_set_dir
        )
        for _ in range(RETRY_RECORD_WPR_COUNT):
          try:
            shell_utils.run(' '.join(record_wpr_cmd), shell=True)

            # Move over the created archive into the local webpages archive
            # directory.
            shutil.move(
              os.path.join(LOCAL_REPLAY_WEBPAGES_ARCHIVE_DIR, wpr_data_file),
              self._local_record_webpages_archive_dir)
            shutil.move(
              os.path.join(LOCAL_REPLAY_WEBPAGES_ARCHIVE_DIR,
                           page_set_json_name),
              self._local_record_webpages_archive_dir)

            # Break out of the retry loop since there were no errors.
            break
          except Exception:
            # There was a failure continue with the loop.
            traceback.print_exc()
        else:
          # If we get here then record_wpr did not succeed and thus did not
          # break out of the loop.
          raise Exception('record_wpr failed for page_set: %s' % page_set)

      else:
        # Get the webpages archive so that it can be replayed.
        self._DownloadWebpagesArchive(wpr_data_file, page_set_json_name)

      run_benchmark_cmd = (
          'PYTHONPATH=%s:%s:$PYTHONPATH' % (page_set_dir, self._catapult_dir),
          'DISPLAY=%s' % X11_DISPLAY,
          'timeout', '300',
          os.path.join(self._telemetry_binaries_dir, 'run_benchmark'),
          '--extra-browser-args="%s"' % self._browser_args,
          '--browser=exact',
          '--browser-executable=%s' % self._browser_executable,
          SKP_BENCHMARK,
          '--page-set-name=%s' % page_set_basename,
          '--page-set-base-dir=%s' % page_set_dir,
          '--skp-outdir=%s' % TMP_SKP_DIR,
          '--also-run-disabled-tests'
      )

      for _ in range(RETRY_RUN_MEASUREMENT_COUNT):
        try:
          print '\n\n=======Capturing SKP of %s=======\n\n' % page_set
          shell_utils.run(' '.join(run_benchmark_cmd), shell=True)
        except shell_utils.CommandFailedException:
          # skpicture_printer sometimes fails with AssertionError but the
          # captured SKP is still valid. This is a known issue.
          pass

        # Rename generated SKP files into more descriptive names.
        try:
          self._RenameSkpFiles(page_set)
          # Break out of the retry loop since there were no errors.
          break
        except Exception:
          # There was a failure continue with the loop.
          traceback.print_exc()
          print '\n\n=======Retrying %s=======\n\n' % page_set
          time.sleep(10)
      else:
        # If we get here then run_benchmark did not succeed and thus did not
        # break out of the loop.
        raise Exception('run_benchmark failed for page_set: %s' % page_set)

    print '\n\n=======Capturing SKP files took %s seconds=======\n\n' % (
        time.time() - start_time)

    if self._skia_tools:
      render_pictures_cmd = [
          os.path.join(self._skia_tools, 'render_pictures'),
          '-r', self._local_skp_dir
      ]
      render_pdfs_cmd = [
          os.path.join(self._skia_tools, 'render_pdfs'),
          '-r', self._local_skp_dir
      ]

      for tools_cmd in (render_pictures_cmd, render_pdfs_cmd):
        print '\n\n=======Running %s=======' % ' '.join(tools_cmd)
        proc = subprocess.Popen(tools_cmd)
        (code, _) = shell_utils.log_process_after_completion(proc, echo=False)
        if code != 0:
          raise Exception('%s failed!' % ' '.join(tools_cmd))

      if not self._non_interactive:
        print '\n\n=======Running debugger======='
        os.system('%s %s' % (os.path.join(self._skia_tools, 'debugger'),
                             self._local_skp_dir))

    print '\n\n'

    if self._upload:
      print '\n\n=======Uploading to %s=======\n\n' % self.gs.target_type()
      # Copy the directory structure in the root directory into Google Storage.
      dest_dir_name = ROOT_PLAYBACK_DIR_NAME
      if self._alternate_upload_dir:
        dest_dir_name = self._alternate_upload_dir

      self.gs.upload_dir_contents(
          LOCAL_PLAYBACK_ROOT_DIR, dest_dir=dest_dir_name,
          upload_if=gs_utils.GSUtils.UploadIf.IF_MODIFIED,
          predefined_acl=GS_PREDEFINED_ACL,
          fine_grained_acl_list=GS_FINE_GRAINED_ACL_LIST)

      print '\n\n=======New SKPs have been uploaded to %s =======\n\n' % (
          posixpath.join(self.gs.target_name(), dest_dir_name,
                         SKPICTURES_DIR_NAME))

      if self._upload_to_partner_bucket:
        print '\n\n=======Uploading to Partner bucket %s =======\n\n' % (
            PARTNERS_GS_BUCKET)
        partner_gs = GoogleStorageDataStore(PARTNERS_GS_BUCKET)
        partner_gs.delete_path(SKPICTURES_DIR_NAME)
        partner_gs.upload_dir_contents(
            os.path.join(LOCAL_PLAYBACK_ROOT_DIR, SKPICTURES_DIR_NAME),
            dest_dir=SKPICTURES_DIR_NAME,
            upload_if=gs_utils.GSUtils.UploadIf.IF_MODIFIED)
        print '\n\n=======New SKPs have been uploaded to %s =======\n\n' % (
            posixpath.join(partner_gs.target_name(), SKPICTURES_DIR_NAME))
    else:
      print '\n\n=======Not Uploading to %s=======\n\n' % self.gs.target_type()
      print 'Generated resources are available in %s\n\n' % (
          LOCAL_PLAYBACK_ROOT_DIR)

    return 0

  def _GetSkiaSkpFileName(self, page_set):
    """Returns the SKP file name for Skia page sets."""
    # /path/to/skia_yahooanswers_desktop.py -> skia_yahooanswers_desktop.py
    ps_filename = os.path.basename(page_set)
    # skia_yahooanswers_desktop.py -> skia_yahooanswers_desktop
    ps_basename, _ = os.path.splitext(ps_filename)
    # skia_yahooanswers_desktop -> skia, yahooanswers, desktop
    _, page_name, device = ps_basename.split('_')
    basename = '%s_%s' % (DEVICE_TO_PLATFORM_PREFIX[device], page_name)
    return basename[:MAX_SKP_BASE_NAME_LEN] + '.skp'

  def _GetChromiumSkpFileName(self, page_set, site):
    """Returns the SKP file name for Chromium page sets."""
    # /path/to/http___mobile_news_sandbox_pt0 -> http___mobile_news_sandbox_pt0
    _, webpage = os.path.split(site)
    # http___mobile_news_sandbox_pt0 -> mobile_news_sandbox_pt0
    for prefix in ('http___', 'https___', 'www_'):
      if webpage.startswith(prefix):
        webpage = webpage[len(prefix):]
    # /path/to/skia_yahooanswers_desktop.py -> skia_yahooanswers_desktop.py
    ps_filename = os.path.basename(page_set)
    # http___mobile_news_sandbox -> pagesetprefix_http___mobile_news_sandbox
    basename = '%s_%s' % (CHROMIUM_PAGE_SETS_TO_PREFIX[ps_filename], webpage)
    return basename[:MAX_SKP_BASE_NAME_LEN] + '.skp'

  def _RenameSkpFiles(self, page_set):
    """Rename generated SKP files into more descriptive names.

    Look into the subdirectory of TMP_SKP_DIR and find the most interesting
    .skp in there to be this page_set's representative .skp.
    """
    subdirs = glob.glob(os.path.join(TMP_SKP_DIR, '*'))
    for site in subdirs:
      if self._IsChromiumPageSet(page_set):
        filename = self._GetChromiumSkpFileName(page_set, site)
      else:
        filename = self._GetSkiaSkpFileName(page_set)
      filename = filename.lower()

      if self._skp_prefix:
        filename = '%s%s' % (self._skp_prefix, filename)

      # We choose the largest .skp as the most likely to be interesting.
      largest_skp = max(glob.glob(os.path.join(site, '*.skp')),
                        key=lambda path: os.stat(path).st_size)
      dest = os.path.join(self._local_skp_dir, filename)
      print 'Moving', largest_skp, 'to', dest
      shutil.move(largest_skp, dest)
      self._skp_files.append(filename)
      shutil.rmtree(site)

  def _CreateLocalStorageDirs(self):
    """Creates required local storage directories for this script."""
    for d in (self._local_record_webpages_archive_dir,
              self._local_skp_dir):
      if os.path.exists(d):
        shutil.rmtree(d)
      os.makedirs(d)

  def _DownloadWebpagesArchive(self, wpr_data_file, page_set_json_name):
    """Downloads the webpages archive and its required page set from GS."""
    wpr_source = posixpath.join(ROOT_PLAYBACK_DIR_NAME, 'webpages_archive',
                                wpr_data_file)
    page_set_source = posixpath.join(ROOT_PLAYBACK_DIR_NAME,
                                     'webpages_archive',
                                     page_set_json_name)
    gs = self.gs
    if (gs.does_storage_object_exist(wpr_source) and
        gs.does_storage_object_exist(page_set_source)):
      gs.download_file(wpr_source,
                       os.path.join(LOCAL_REPLAY_WEBPAGES_ARCHIVE_DIR,
                                    wpr_data_file))
      gs.download_file(page_set_source,
                       os.path.join(LOCAL_REPLAY_WEBPAGES_ARCHIVE_DIR,
                                    page_set_json_name))
    else:
      raise Exception('%s and %s do not exist in %s!' % (gs.target_type(),
        wpr_source, page_set_source))

class DataStore:
  """An abstract base class for uploading recordings to a data storage.
  The interface emulates the google storage api."""
  def target_name(self):
    raise NotImplementedError()
  def target_type(self):
    raise NotImplementedError()
  def does_storage_object_exist(self, *args):
    raise NotImplementedError()
  def download_file(self, *args):
    raise NotImplementedError()
  def upload_dir_contents(self, source_dir, **kwargs):
    raise NotImplementedError()

class GoogleStorageDataStore(DataStore):
  def __init__(self, data_store_url):
    self._data_store_url = data_store_url
    self._bucket = remove_prefix(self._data_store_url.lstrip(),
                                 gs_utils.GS_PREFIX)
    self.gs = gs_utils.GSUtils()
  def target_name(self):
    return self._data_store_url
  def target_type(self):
    return 'Google Storage'
  def does_storage_object_exist(self, *args):
    return self.gs.does_storage_object_exist(self._bucket, *args)
  def delete_path(self, path):
    _, files = self.gs.list_bucket_contents(self._bucket, subdir=path)
    for f in files:
      self.gs.delete_file(self._bucket, posixpath.join(path, f))
  def download_file(self, *args):
    self.gs.download_file(self._bucket, *args)
  def upload_dir_contents(self, source_dir, **kwargs):
    self.gs.upload_dir_contents(source_dir, self._bucket, **kwargs)

class LocalFileSystemDataStore(DataStore):
  def __init__(self, data_store_location):
    self._base_dir = data_store_location
  def target_name(self):
    return self._base_dir
  def target_type(self):
    return self._base_dir
  def does_storage_object_exist(self, name, *args):
    return os.path.isfile(os.path.join(self._base_dir, name))
  def delete_path(self, path):
    shutil.rmtree(path)
  def download_file(self, name, local_path, *args):
    shutil.copyfile(os.path.join(self._base_dir, name), local_path)
  def upload_dir_contents(self, source_dir, dest_dir, **kwargs):
    def copytree(source_dir, dest_dir):
      if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
      for item in os.listdir(source_dir):
        source = os.path.join(source_dir, item)
        dest = os.path.join(dest_dir, item)
        if os.path.isdir(source):
          copytree(source, dest)
        else:
          shutil.copy2(source, dest)
    copytree(source_dir, os.path.join(self._base_dir, dest_dir))

if '__main__' == __name__:
  option_parser = optparse.OptionParser()
  option_parser.add_option(
      '', '--page_sets',
      help='Specifies the page sets to use to archive. Supports globs.',
      default='all')
  option_parser.add_option(
      '', '--record', action='store_true',
      help='Specifies whether a new website archive should be created.',
      default=False)
  option_parser.add_option(
      '', '--skia_tools',
      help=('Path to compiled Skia executable tools. '
            'render_pictures/render_pdfs is run on the set '
            'after all SKPs are captured. If the script is run without '
            '--non-interactive then the debugger is also run at the end. Debug '
            'builds are recommended because they seem to catch more failures '
            'than Release builds.'),
      default=None)
  option_parser.add_option(
      '', '--upload', action='store_true',
      help=('Uploads to Google Storage or copies to local filesystem storage '
            ' if this is True.'),
      default=False)
  option_parser.add_option(
      '', '--upload_to_partner_bucket', action='store_true',
      help=('Uploads SKPs to the chrome-partner-telemetry Google Storage '
            'bucket if true.'),
      default=False)
  option_parser.add_option(
      '', '--data_store',
    help=('The location of the file storage to use to download and upload '
          'files. Can be \'gs://<bucket>\' for Google Storage, or '
          'a directory for local filesystem storage'),
      default='gs://chromium-skia-gm')
  option_parser.add_option(
      '', '--alternate_upload_dir',
      help= ('Uploads to a different directory in Google Storage or local '
             'storage if this flag is specified'),
      default=None)
  option_parser.add_option(
      '', '--output_dir',
      help=('Temporary directory where SKPs and webpage archives will be '
            'outputted to.'),
      default=tempfile.gettempdir())
  option_parser.add_option(
      '', '--browser_executable',
      help='The exact browser executable to run.',
      default=None)
  option_parser.add_option(
      '', '--browser_extra_args',
      help='Additional arguments to pass to the browser.',
      default=None)
  option_parser.add_option(
      '', '--chrome_src_path',
      help='Path to the chromium src directory.',
      default=None)
  option_parser.add_option(
      '', '--non-interactive', action='store_true',
      help='Runs the script without any prompts. If this flag is specified and '
           '--skia_tools is specified then the debugger is not run.',
      default=False)
  option_parser.add_option(
      '', '--skp_prefix',
      help='Prefix to add to the names of generated SKPs.',
      default=None)
  options, unused_args = option_parser.parse_args()

  playback = SkPicturePlayback(options)
  sys.exit(playback.Run())
