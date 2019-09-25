#
# update_sdk_and_toolchain.py <sdk dir> <clang dir>
#
#    Downloads both the latest Fuchsia SDK and Fuchsia-compatible clang
#    zip archives from chrome infra (CIPD) and extracts them to
#    <sdk dir> and <clang dir> respectively.  This provides the complete
#    toolchain required to build Fuchsia binaries from the Fuchsia SDK.
#
import argparse
import logging
import os
import platform
import shutil
import subprocess
import tempfile

def MessageExit(message):
  logging.error(message)
  sys.exit(1)


def CipdLives():
    err_msg = "Cipd not found, please install. See: " + \
              "https://commondatastorage.googleapis.com/chrome-infra-docs/flat" + \
              "/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up"
    try:
        subprocess.call(["cipd", "--version"])
    except OSError as e:
        if e.errno == errno.ENOENT:
            MessageExit(err_msg)
        else:
            MessageExit("cipd command execution failed.")

def DownloadIt(pkg_name, cipd_cache_dir, output_dir):
  # prefix = pkg_name.replace('/', '-')
  # temp_file = tempfile.NamedTemporaryFile(prefix=prefix, suffix=".zip")
  temp_file = "/tmp/" + pkg_name.replace('/','-') + ".zip"
  cipd_cmd = "cipd pkg-fetch " + pkg_name + " -version latest -out " + \
      temp_file + " -cache-dir " + cipd_cache_dir
      # temp_file.name + " -cache-dir " + cipd_cache_dir
  print "\n" + cipd_cmd + "\n"
  os.system(cipd_cmd)
  unzip_cmd = "unzip -q " + temp_file + " -d " + output_dir
  print unzip_cmd + "\n"
  os.system(unzip_cmd)

def Main():
  CipdLives()
  parser = argparse.ArgumentParser()
  parser.add_argument("-sdk_dir", type=str,
          help="Destination directory for the fuchsia SDK.")
  parser.add_argument("-clang_dir", type=str,
          help="Destination directory for the fuchsia toolchain.")
  parser.add_argument("-overwrite_dirs", type=bool, default=False,
          help="REMOVES EXISTING sdk AND clang DIRS and makes new ones.")
  parser.add_argument("-cipd_cache_dir", type=str, default="/tmp", required=False,
          help="Cache directory for CIPD downloads to prevent redundant downloads.")
  args = parser.parse_args()
  
  sdk_dir = args.sdk_dir
  clang_dir = args.clang_dir
  if args.overwrite_dirs:
    dirs = [ sdk_dir, clang_dir ]
    for dir in dirs:
      try:
        if os.path.exists(dir):
            shutil.rmtree(dir)
        os.makedirs(dir)
      except OSError:
        MessageExit("Creation of the directory %s failed" % dir)
  else:
    if not os.path.exists(sdk_dir):
        os.makedirs(sdk_dir)
    if not os.path.exists(clang_dir):
        os.makedirs(clang_dir)
    if (not os.access(sdk_dir, os.W_OK)) or (not os.path.isdir(sdk_dir)):
      MessageExit("Can't write to sdk dir " + sdk_dir)
    if (not os.access(clang_dir, os.W_OK)) or (not os.path.isdir(clang_dir)):
      MessageExit("Can't write to clang dir " + clang_dir) 
  
  ostype = platform.system()
  if ostype == "Linux":
    os_string = "linux-amd64"
  elif ostype == "Darwin":
    os_string = "mac-amd64"
  else:
    MessageExit("Unknown host " + ostype)
  
  sdk_pkg = "fuchsia/sdk/core/" + os_string
  DownloadIt(sdk_pkg, args.cipd_cache_dir, sdk_dir)
  clang_pkg = "fuchsia/clang/" + os_string
  DownloadIt(clang_pkg, args.cipd_cache_dir, clang_dir)

if __name__ == "__main__":
  import sys
  Main()
