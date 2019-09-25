#
# update_sdk_and_toolchain.py <sdk dir> <clang dir>
#
#    Downloads both the latest Fuchsia SDK and Fuchsia-compatible clang
#    zip archives from chrome infra (CIPD) and extracts them to
#    <sdk dir> and <clang dir> respectively.  This provides the complete
#    toolchain required to build Fuchsia binaries from the Fuchsia SDK.
#
import argparse
import os
import platform
import shutil
import tempfile

def MessageExit(message):
  print message
  exit(1)

def DownloadIt(prefix, path, archive_suffix, output_dir):
  temp_file = tempfile.NamedTemporaryFile(prefix=prefix, suffix=".zip")
  cipd_base_url = "https://chrome-infra-packages.appspot.com/dl/fuchsia"
  curl_cmd = "curl -q -# -L " + cipd_base_url + path + archive_suffix + " -o " + temp_file.name
  print curl_cmd
  os.system(curl_cmd)
  unzip_cmd = "unzip -q " + temp_file.name + " -d " + output_dir
  os.system(unzip_cmd)

def Main():
  parser = argparse.ArgumentParser()
  parser.add_argument("-sdk_dir", type=str, help="Destination directory for the fuchsia SDK.")
  parser.add_argument("-clang_dir", type=str, help="Destination directory for the fuchsia toolchain.")
  parser.add_argument("-overwrite_dirs", type=bool, default=True, help="REMOVES EXISTING sdk AND clang DIRS and makes new ones.")
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
    if (not os.access(sdk_dir, os.W_OK)) or (not os.path.isdir(sdk_dir)):
      MessageExit("Can't write to sdk dir " + sdk_dir)
    if (not os.access(clang_dir, os.W_OK)) or (not os.path.isdir(clang_dir)):
      MessageExit("Can't write to clang dir " + clang_dir) 
  
  ostype = platform.system()
  if ostype == "Linux":
    archive_suffix = "linux-amd64/+/latest"
  elif ostype == "Darwin":
    archive_suffix = "mac-amd64/+/latest"
  else:
    MessageExit("Unsupported host " + ostype)
  
  DownloadIt("fuchsia_sdk", "/sdk/core/", archive_suffix, sdk_dir)
  DownloadIt("fuchsia_clang", "/clang/", archive_suffix, clang_dir)

if __name__ == "__main__":
  import sys
  Main()
