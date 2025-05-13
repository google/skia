#!/usr/bin/env python3
# Copyright 2025 Google LLC
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# It is a pain to tell Bazel "check out this repo and build it with these
# other external files". We can't have something like @freetype use files
# from @freetype_config because the include paths don't work. We can't
# copy the files from @freetype_config into @freetype first via a genrule
# without having external clients need to do some very janky things to set
# up the dependencies correctly. A modified repo_rule didn't work either because
# the absolute file paths got trapped in the MODULE.bazel.lock and kjlubick@
# couldn't figure out how to get relative file paths in there to work.
#
# The least bad option is to combine any files we want to add as a "patch"
# and then apply the patch via the git_repository_rule when we check it out.
# This Python script takes in one or more pairs of "source file in Skia" and
# "dst path in other repo" and makes a patch with all those contents.
# This file will need to be re-run (see //bazel/Makefile:generate_third_party_patches)
# if we update those config files to keep things in sync, which is not ideal
# but it works with the way things are.

import sys

def create_single_file_patch(source_file, destination_path):
    try:
        with open(source_file, 'r') as f:
            content = f.read()
    except FileNotFoundError:
        print(f"Error: Source file not found: {source_file}", file=sys.stderr)
        return None

    lines = content.splitlines()
    line_count = len(lines)

    # Construct the patch section with the new header lines
    # of note, the bazel patch tool doesn't seem to care about the index
    # (git would normally have the sha1 sum of the file contents plus a header)
    # so we can just use a stub and it works fine.
    patch_section = f"new file mode 100644\n" \
                    f"index 0000000..fffffff\n" \
                    f"--- /dev/null\n" \
                    f"+++ {destination_path}\n" \
                    f"@@ -0,0 +1,{line_count} @@\n"

    for line in lines:
        patch_section += "+" + line + "\n"
    return patch_section


if __name__ == "__main__":
    if (len(sys.argv) - 1) % 2 != 0 or len(sys.argv) < 3:
        print("Usage: python3 generate_patch.py <source_file1> <destination_path1> [<source_file2> <destination_path2> ...]", file=sys.stderr)
        sys.exit(1)

    all_patch_content = []
    # sys.argv[0] is the script name, so start from index 1 and step by 2
    for i in range(1, len(sys.argv), 2):
        source_file = sys.argv[i]
        destination_path = sys.argv[i + 1]

        patch_section = create_single_file_patch(source_file, destination_path)
        if patch_section is None:
            sys.exit(1)
        all_patch_content.append(patch_section)

    print("".join(all_patch_content))
