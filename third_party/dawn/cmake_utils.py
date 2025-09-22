#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
import hashlib
import os
import shutil
import subprocess
import sys

def discover_dependencies(build_dir, targets):
  """Runs ninja -tinputs recursively to discover all targets, then uses
  ninja -tdeps to get all source files and object files for those targets.
  Returns a tuple of (list of source files, list of object files)."""
  # The worklist contains ninja targets we need to find the inputs for.
  worklist = set(targets)
  # We keep track of all ninja targets we've ever seen to avoid cycles and
  # redundant processing.
  seen_targets = set(targets)

  BATCH_SIZE = 100 if sys.platform == 'win32' else 1000

  while len(worklist) > 0:
    current_batch = list(worklist)[:BATCH_SIZE]
    worklist = worklist.difference(current_batch)

    cmd = ["ninja", "-C", build_dir, "-tinputs"] + current_batch
    inputs = subprocess.check_output(cmd).decode("utf-8").splitlines()
    # inputs looks like:
    #   /home/user/skia/third_party/externals/dawn/src/tint/utils/text/styled_text_theme.cc
    #   /home/user/skia/third_party/externals/dawn/src/tint/utils/text/unicode.cc
    #   cmake_object_order_depends_target_tint_api_common
    #   cmake_object_order_depends_target_tint_lang_core
    #   src/tint/CMakeFiles/tint_api_common.dir/api/common/vertex_pulling_config.cc.o
    #   src/tint/CMakeFiles/tint_lang_core.dir/lang/core/binary_op.cc.o
    #   src/tint/libtint_api_common.a
    #   src/tint/libtint_lang_core.a
    # Some of these are files (which we can ignore), some are more targets, which we should
    # recursively get targets for to make sure we have the whole dependency graph.
    for line in inputs:
      line = line.strip()
      if line.startswith("/"):
        continue  # file, which are always absolute paths, skip it
      if line not in seen_targets:
        worklist.add(line)
        seen_targets.add(line)

  object_files = []
  for target in seen_targets:
    if target.endswith(".o") or target.endswith(".obj"):
      object_files.append(target)
  object_files.sort()

  # Now that we have all the targets, get the dependencies for them.
  source_files = set()
  all_targets = list(seen_targets)
  for i in range(0, len(all_targets), BATCH_SIZE):
    chunk = all_targets[i:i + BATCH_SIZE]
    cmd = ["ninja", "-C", build_dir, "-tdeps"] + chunk
    output = subprocess.check_output(cmd).decode("utf-8").splitlines()

    # When a target has deps, which are read from the .d files generated from the "-dkeepdepfile
    # option earlier, the ninja command outputs something like this:
    #   third_party/spirv-tools/source/opt/CMakeFiles/SPIRV-Tools-opt.dir/eliminate_dead_functions_util.cpp.o: #deps 408, deps mtime 1755004530688332629 (VALID)
    #       /home/user/skia/third_party/externals/dawn/third_party/spirv-tools/src/source/opt/eliminate_dead_functions_util.cpp
    #       /usr/include/stdc-predef.h
    #       /home/user/skia/third_party/externals/dawn/third_party/spirv-tools/src/source/opt/eliminate_dead_functions_util.h
    #       /home/user/skia/third_party/externals/dawn/third_party/spirv-tools/src/source/opt/ir_context.h
    #       /usr/include/c++/14/algorithm
    # If there's not a match, it's a simple message like:
    # src/tint/libtint_utils_text_generator.a: deps not found
    # We want to aggregate all the indented files
    for line in output:
      if not line.startswith("  ") or "deps not found" in line:
        continue
      line = line.strip()
      source_files.add(line)

  result = list(source_files)
  result.sort()
  return result, object_files


def write_depfile(output_path, depfile_path, dependencies):
  """Generates a .d file that lists all discovered source files as
  dependencies for the given output library."""
  os.makedirs(os.path.dirname(depfile_path), exist_ok=True)
  with open(depfile_path, "w") as f:
    f.write(f"{output_path}:")
    # Make sure to use forward slashes for paths in the depfile, even on
    # Windows, for consistency.
    for dep in dependencies:
      f.write(" \\\n" + dep.replace("\\", "/"))
    f.write("\n")


def quote_if_needed(path):
  """Adds quotes to a path if it contains spaces."""
  if " " in path:
    return f'"{path}"'
  return path


def copy_if_changed(src, dest):
  """Copies the file from src to dest if dest doesn't exist or if the
  contents of src and dest are different."""
  if os.path.exists(dest):
    src_hash = hashlib.sha256(open(src, "rb").read()).hexdigest()
    dest_hash = hashlib.sha256(open(dest, "rb").read()).hexdigest()
    if src_hash == dest_hash:
      # The files are identical, no need to copy.
      return

  # Either the destination does not exist or it is different.
  shutil.copyfile(src, dest)
  os.chmod(dest, 0o755)
