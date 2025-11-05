#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
import argparse
import hashlib
import os
import shutil
import subprocess
import sys


def add_common_cmake_args(parser):
  """Adds common arguments used for building with CMake."""
  print(f"Running with Python executable: {sys.executable}")

  parser.add_argument("--cc", required=True, help="Path to the C compiler.")
  parser.add_argument("--cxx", required=True, help="Path to the C++ compiler.")
  parser.add_argument(
      "--cxx_flags",
      default=[],
      action="append",
      help="C++ compiler flags. Can be specified multiple times.")
  parser.add_argument(
      "--ld_flags",
      default=[],
      action="append",
      help="Linker flags. Can be specified multiple times.")
  parser.add_argument(
      "--output_path", required=True, help="Path to the output library.")
  parser.add_argument(
      "--depfile_path", required=True, help="Path to the depfile to generate.")
  parser.add_argument(
      "--target_os", required=True, help="Target OS for cross-compilation.")
  parser.add_argument(
      "--target_cpu", required=True, help="Target CPU for cross-compilation.")
  parser.add_argument(
      "--win_sdk", default="", help="Path to the Windows SDK.")
  parser.add_argument(
      "--win_sdk_version", default="", help="Version of the Windows SDK.")
  parser.add_argument(
      "--win_vc", default="", help="Path to Visual C++.")
  parser.add_argument(
      "--win_toolchain_version",
      default="",
      help="Version of the MSVC toolchain.")
  parser.add_argument(
      "--build_type", default="Release", help="CMake build type.")
  parser.add_argument(
      "--build_dir",
      required=True,
      help="A short name for the build directory.")
  parser.add_argument("--is_clang", action=argparse.BooleanOptionalAction)
  parser.add_argument(
      "--enable_rtti", action=argparse.BooleanOptionalAction, help="Enable RTTI.")

def add_next_batch_to_command(base_cmd, workset):
  if sys.platform != 'win32':
    batch = list(workset)
    return base_cmd + batch, batch
  # Windows has a limit of about 8100 characters on command line commands.
  # Thus we batch our commands to fit under this. 100 is usually short enough
  # but with fluctuations on the order returned by ninja and set(), this can
  # still sometimes be too long, so we ensure we don't go over the limit.
  BATCH_SIZE = 100
  batch = list(workset)[:BATCH_SIZE]
  cmd = base_cmd + batch

  while len(' '.join(cmd)) > 8100:
    assert(len(batch) > 1)
    batch.pop()
    cmd = base_cmd + batch

  return cmd, batch

def discover_dependencies(build_dir, targets):
  """Runs ninja -tinputs recursively to discover all targets, then uses
  ninja -tdeps to get all source files and object files for those targets.
  Returns a tuple of (list of source files, list of object files)."""
  # The worklist contains ninja targets we need to find the inputs for.
  worklist = set(targets)
  # We keep track of all ninja targets we've ever seen to avoid cycles and
  # redundant processing.
  seen_targets = set(targets)

  source_files = set()

  ninja = shutil.which("ninja");
  if not ninja:
    print("Error: ninja not found in PATH.")
    sys.exit(1)

  while len(worklist) > 0:
    cmd, current_batch = add_next_batch_to_command([ninja, "-C", build_dir, "-tinputs"],
                                                   worklist)
    worklist = worklist.difference(current_batch)

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
    # Some of these are files (which we can add to the source file list), some are more targets,
    # which we should recursively get targets for to make sure we have the whole dependency graph.
    for line in inputs:
      line = line.strip()
      if os.path.isabs(line):
        # Absolute paths are source files.
        source_files.add(line)
        continue
      if line not in seen_targets:
        worklist.add(line)
        seen_targets.add(line)

  object_files = []
  for target in seen_targets:
    if target.endswith(".o") or target.endswith(".obj"):
      object_files.append(target)
  object_files.sort()

  # Now that we have all the targets and some of the source files, get the dependencies for them.
  abs_build_dir = os.path.abspath(build_dir)

  worklist = set(seen_targets)
  while len(worklist) > 0:
    cmd, current_batch = add_next_batch_to_command([ninja, "-C", build_dir, "-tdeps"],
                                                   worklist)
    worklist = worklist.difference(current_batch)

    output = subprocess.check_output(cmd).decode("utf-8").splitlines()
    # When a target has deps, which are read from the .d files generated from the "-dkeepdepfile
    # option earlier, the ninja command outputs something like this:
    #   third_party/spirv-tools/source/opt/CMakeFiles/SPIRV-Tools-opt.dir/eliminate_dead_functions_util.cpp.o: #deps 408, deps mtime 1755004530688332629 (VALID)
    #       /home/user/skia/third_party/externals/dawn/third_party/spirv-tools/src/source/opt/eliminate_dead_functions_util.cpp
    #       /usr/include/stdc-predef.h
    #       /home/user/skia/third_party/externals/dawn/third_party/spirv-tools/src/source/opt/eliminate_dead_functions_util.h
    #       /home/user/skia/third_party/externals/dawn/third_party/spirv-tools/src/source/opt/ir_context.h
    #       /usr/include/c++/14/algorithm
    #   These can sometimes be relative files (on Windows) with the base path being the build_dir
    #   src/tint/CMakeFiles/tint_lang_wgsl_sem.dir/lang/wgsl/sem/variable.cc.obj: #deps 300, deps mtime 7816349331123163 (VALID)
    #     ../../../../../cipd/clang_win/lib/clang/18/include/x86gprintrin.h
    #     ../../../../../cipd/win_toolchain/VC/Tools/MSVC/14.39.33519/include/__msvc_bit_utils.hpp
    #     ../../../../../cipd/win_toolchain/VC/Tools/MSVC/14.39.33519/include/cctype
    #     ../../../../../cipd/win_toolchain/win_sdk/Include/10.0.22621.0/ucrt/wchar.h
    #     ../../../third_party/externals/dawn/src/tint/api/common/binding_point.h
    #     ../../../third_party/externals/dawn/src/tint/lang/core/constant/clone_context.h
    # If there's not a match, it's a simple message like:
    # src/tint/libtint_utils_text_generator.a: deps not found
    # We want to aggregate all the indented files
    for line in output:
      if not line.startswith("  ") or "deps not found" in line:
        continue
      line = line.strip()
      if os.path.isabs(line):
        source_files.add(line)
      else:
        dep = os.path.normpath(os.path.join(abs_build_dir, line))
        source_files.add(dep)

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


def get_cmake_os_cpu(os, cpu):
  # https://stackoverflow.com/a/70498851
  os = os.lower().strip()
  cpu = cpu.lower().strip()
  if os == "android":
    target_cpu_map = {
      "arm": "armeabi-v7a",
      "arm64": "arm64-v8a",
      "x64": "x86_64",
      "x86": "x86",
    }
    return "Android", target_cpu_map[cpu]

  if os == "linux":
    target_cpu_map = {
      "arm": "arm",
      "arm64": "aarch64",
      "x64": "x86_64",
      "x86": "i686",
    }
    return "Linux", target_cpu_map[cpu]

  if os == "mac":
    target_cpu_map = {
      "arm64": "arm64",
      "x64": "x86_64",
    }
    return "Darwin", target_cpu_map[cpu]

  if os == "win":
    target_cpu_map = {
      "arm64": "ARM64",
      "x64": "AMD64",
    }
    return "Windows", target_cpu_map[cpu]

  print("Unsupported OS")
  sys.exit(1)


def get_windows_settings(args):
  """The Windows toolchain requires a lot of setup for cmake to use it.
     This encapsulates all that setup.
  """

  assert(args.win_vc)
  assert(args.win_sdk)
  assert(args.win_sdk_version)

  win_cfgs, win_cxx, win_ld  = [], [], []

  # Set the Windows SDK version for CMake.
  win_cfgs.append(
      f"-DCMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION={args.win_sdk_version}")

  # Explicitly tell CMake where to find the Resource Compiler, Manifest Tool, and Archiver.
  rc_exe_path = os.path.join(args.win_sdk, "bin", args.win_sdk_version,
                             args.target_cpu, "rc.exe")
  win_cfgs.append(f"-DCMAKE_RC_COMPILER={quote_if_needed(rc_exe_path.replace(os.sep, '/'))}")
  mt_exe_path = os.path.join(args.win_sdk, "bin", args.win_sdk_version,
                             args.target_cpu, "mt.exe")
  win_cfgs.append(f"-DCMAKE_MT={quote_if_needed(mt_exe_path.replace(os.sep, '/'))}")

  ar_exe_path = os.path.join(args.win_vc, "Tools", "MSVC",
                             args.win_toolchain_version, "bin", "Hostx64",
                             args.target_cpu, "lib.exe")
  win_cfgs.append(f"-DCMAKE_AR={quote_if_needed(ar_exe_path.replace(os.sep, '/'))}")

  # On Windows, we need to explicitly tell clang where to find the toolchain
  # headers and libraries.
  um_lib_path = os.path.join(args.win_sdk, "Lib", args.win_sdk_version, "um", args.target_cpu)
  ucrt_lib_path = os.path.join(args.win_sdk, "Lib", args.win_sdk_version, "ucrt", args.target_cpu)
  msvc_lib_path = os.path.join(args.win_vc, "Tools", "MSVC", args.win_toolchain_version, "lib", args.target_cpu)

  win_ld += [
    f"/LIBPATH:{quote_if_needed(um_lib_path.replace(os.sep, '/'))}",
    f"/LIBPATH:{quote_if_needed(ucrt_lib_path.replace(os.sep, '/'))}",
    f"/LIBPATH:{quote_if_needed(msvc_lib_path.replace(os.sep, '/'))}",
  ]

  # Skia builds with exceptions and RTTI disabled so we must also build Dawn that way.
  win_cxx += [
      "-D_HAS_EXCEPTIONS=0",
      "/GR-",
      "/w",  # Dawn's warnings are noisy
  ]

  # Skia uses a hermetic toolchain, so we need to tell clang where to
  # find the MSVC headers and libraries. If we pass the MSVC style flags (/I)
  # to clang, then abseil fails to compile with errors about using a
  # reinterpret_cast in a static_assert.
  if args.is_clang:
    win_cxx += [
        "-imsvc",
        os.path.join(args.win_vc, "Tools", "MSVC", args.win_toolchain_version, "include"),
        "-imsvc",
        os.path.join(args.win_sdk, "Include", args.win_sdk_version, "ucrt"),
        "-imsvc",
        os.path.join(args.win_sdk, "Include", args.win_sdk_version, "shared"),
        "-imsvc",
        os.path.join(args.win_sdk, "Include", args.win_sdk_version, "um"),
        "-imsvc",
        os.path.join(args.win_sdk, "Include", args.win_sdk_version, "winrt"),
    ]
  else:
    win_cxx += [
        "/I" + os.path.join(args.win_vc, "Tools", "MSVC", args.win_toolchain_version, "include"),
        "/I" + os.path.join(args.win_sdk, "Include", args.win_sdk_version, "ucrt"),
        "/I" + os.path.join(args.win_sdk, "Include", args.win_sdk_version, "shared"),
        "/I" + os.path.join(args.win_sdk, "Include", args.win_sdk_version, "um"),
        "/I" + os.path.join(args.win_sdk, "Include", args.win_sdk_version, "winrt"),
    ]

  # We want to build Dawn (and its dependencies) with /MT so we can statically
  # link it into Skia.
  win_cfgs.append("-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded")
  win_cfgs.append("-DABSL_MSVC_STATIC_RUNTIME=ON")

  return win_cfgs, win_cxx, win_ld


def combine_into_library(args, output_path, build_dir, target_os, object_files):
  """Combine all the object files into a single .a/.lib file so it's easier to
     give this to GN."""

  # Delete generated library if it exists already (otherwise ar sometimes chokes)
  lib_name = os.path.basename(output_path)
  gen_library_path = os.path.join(build_dir, lib_name)
  if os.path.exists(gen_library_path):
    os.remove(gen_library_path)

  assert len(object_files) > 0
  # Use ar/lib to join all the object files that comprise the necessary
  # libraries and any transitive dependencies into one .a file.
  if target_os == "Windows":
    # On Windows, we use lld-link.exe for clang, and lib.exe for MSVC.
    if args.is_clang:
        linker_exe = os.path.join(os.path.dirname(args.cc), "lld-link.exe")
    else:
        # We can't just use ar.exe because it is not shipped with MSVC.
        # We must use lib.exe, which has a different command line.
        linker_exe = os.path.join(args.win_vc, "Tools", "MSVC",
                               args.win_toolchain_version, "bin", "Hostx64",
                               args.target_cpu, "lib.exe")
    # The command line can be too long, so we use a response file.
    response_file_name = "objects.rsp"
    response_file_path = os.path.join(build_dir, response_file_name)
    with open(response_file_path, "w") as f:
      for obj in object_files:
        f.write(f'"{obj}"\n')
    combine_obj_cmd = [
        linker_exe, "/LIB", f"/OUT:{lib_name}", f"@{response_file_name}"
    ]
  else:
    combine_obj_cmd = ["ar", "rcs", lib_name] + object_files
  subprocess.run(combine_obj_cmd, cwd=build_dir, check=True)

  copy_if_changed(gen_library_path, os.path.join(os.getcwd(), output_path))



def get_third_party_locations():
  """Return CMake configure arguments to point to or disable third_party deps"""
  def verify_and_get(subpath):
    third_party_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "externals")
    third_party_dir = os.path.abspath(third_party_dir)
    path = os.path.join(third_party_dir, subpath)
    if not os.path.exists(path):
      print(f"Third party path {path} not found - did you sync your DEPS?")
      sys.exit(1)
    return path

  return [
    # Actually downloading the 3p repos is handled by DEPS / tools/git-sync-deps
    "-DDAWN_FETCH_DEPENDENCIES=OFF",
    # Necessary 3p deps
    f"-DDAWN_ABSEIL_DIR={verify_and_get('abseil-cpp')}",
    f"-DDAWN_EGL_REGISTRY_DIR={verify_and_get('egl-registry')}",
    f"-DDAWN_GLSLANG_DIR={verify_and_get('glslang')}",
    f"-DDAWN_JINJA2_DIR={verify_and_get('jinja2')}",
    f"-DDAWN_MARKUPSAFE_DIR={verify_and_get('markupsafe')}",
    f"-DDAWN_OPENGL_REGISTRY_DIR={verify_and_get('opengl-registry')}",
    f"-DDAWN_SPIRV_HEADERS_DIR={verify_and_get('spirv-headers')}",
    f"-DDAWN_SPIRV_TOOLS_DIR={verify_and_get('spirv-tools')}",
    f"-DDAWN_VULKAN_HEADERS_DIR={verify_and_get('vulkan-headers')}",
    f"-DDAWN_VULKAN_UTILITY_LIBRARIES_DIR={verify_and_get('vulkan-utility-libraries')}",
    f"-DDAWN_WEBGPU_HEADERS_DIR={verify_and_get('webgpu-headers')}",
    f"-DDAWN_SWIFTSHADER_DIR={verify_and_get('swiftshader')}",

    # Disable unnecessary deps
    "-DDAWN_BUILD_BENCHMARKS=OFF",
    "-DDAWN_BUILD_PROTOBUF=OFF",
    "-DDAWN_BUILD_SAMPLES=OFF",
    "-DDAWN_BUILD_TESTS=OFF",
    "-DDAWN_USE_GLFW=OFF",
    "-DTINT_BUILD_BENCHMARKS=OFF",
    "-DTINT_BUILD_IR_BINARY=OFF",
    "-DTINT_BUILD_TESTS=OFF",
    "-DDAWN_USE_X11=OFF",

    # Explicitly mark third_party deps as not here to make debugging easier
    "-DDAWN_EMDAWNWEBGPU_DIR=NOT_SYNCED_BY_SKIA",
    "-DDAWN_GLFW_DIR=NOT_SYNCED_BY_SKIA",
    "-DDAWN_LPM_DIR=NOT_SYNCED_BY_SKIA",
    "-DDAWN_PROTOBUF_DIR=NOT_SYNCED_BY_SKIA",
  ]
