#!/usr/bin/python

'''
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

import os
import optparse
import posixpath
import re


def is_ignored(full_path, ignore_list):
  for ignore_path in ignore_list:
    if re.search(ignore_path, full_path, re.I):
      return True
  return False


def find_header_files(include_dirs, ignore_list):
  """Return a list of all '.h' files in top_dir.

  Args:
      include_dirs: Paths to the directories within which to recursively search
          for files ending in '.h'
      ignore_list: Paths to both files and directories that are to be excluded
          from the search for headers

  Returns:
      A list of all the files inside include_dirs that end in '.h', relative to
          their respective include_dir that are not explicitly ignored.
  """
  headers = []
  for top_dir in include_dirs:
    for filename in os.listdir(top_dir):
      full_path = posixpath.join(top_dir, filename)
      if is_ignored(full_path, ignore_list):
        continue
      elif os.path.isdir(full_path):
        nested_headers = find_header_files([full_path], ignore_list)
        for nested_header in nested_headers:
          headers.append(os.path.join(filename, nested_header))
      elif filename.endswith('.h'):
        headers.append(filename)
  return headers


def GenerateIncludeCPP(output_file, include_dirs, ignore_list):
  headers = find_header_files(include_dirs, ignore_list)

  # Emit resulting source file.
  with open(os.path.join(os.getcwd(), output_file), "w+") as output:
    for header in headers:
      output.write("#include <%s>\n" % header)


def main():
  parser = optparse.OptionParser()
  parser.add_option("--ignore", action="store", type="string", dest="ignore",
                    help="file to write the processed sources array to.")
  parser.set_usage("""generate_include_cpp out.cpp include_dir
      out.cpp: C++ code to be generated.
      include_dirs: directories to traverse for include files""")
  (options, args) = parser.parse_args()

  # The MSVS gyp generator uses windows path separators so we intercept those
  # strings and normalize them to our expected posix representation
  include_dirs = []
  for include_dir in args[1:]:
    include_dirs.append(include_dir.replace("\\", "/"))
  ignore_list = options.ignore.replace("\\", "/")

  # We can strip off the relative portion of the path to ensure that when we
  # compare for regex matches we don't fail based on relative path depth
  ignore_list = ignore_list.replace("../", "")

  GenerateIncludeCPP(args[0], include_dirs, ignore_list.split())


if __name__ == "__main__":
  main()
