# Skia Release Notes

This directory contains release notes for the upcoming milestone.

All Markdown (`*.md`) files in this directory are treated as release notes -
except this file (`README.md`). As part of the release process, the contents of
these files will be inserted into `RELEASE_NOTES.md`, and these files will be
deleted.

The release branch tool, which automatically does this aggregation, is described
more thoroughly in https://skia.googlesource.com/buildbot/+/refs/heads/main/sk/.

## Markdown Support

Notes are free to use nearly all of the Markdown language. However, because they
will be inserted into a larger release note file, certain guidelines should be
followed.

1. Do not reference any local files in the `relnotes` directory.

   So nothing like `![Tooltip](image.png)`.

   References to URLs are allowed.
2. Do not use [headings](https://www.markdownguide.org/basic-syntax/#headings).
   Milestones in the top level release notes file (RELEASE_NOTES.md) use a
   single heading per milestone.
3. Do not use start your note with an
   [asterisk](https://www.markdownguide.org/basic-syntax/#unordered-lists) or
   other leading marks. These are automatically inserted by the script.
4. Horizontal rules will automatically be inserted between milestones when the
   release notes are generated
