Multiple repo Chromium trybots
==============================

When a proposed Skia change will require a change in Chromium or Blink it is
often helpful to locally create the Chromium and Blink changes and test with the
proposed Skia change. This often happens with Skia API changes and changes
which affect Blink layout tests. While simple to do locally, this explains how
to do so on the Chromium trybots.

Skia only changes
-----------------
If the Skia patch is already in Rietveld and there are no associated Chromium
changes, then it is possible to just run the Chromium trybots. This will apply
the Skia patch and run the bot.

Skia and Chromium changes
-------------------------
If the Skia patch is already in Rietveld and there are associated Chromium
changes, then in the Chromium CL add the following to
\<chromium>/src/DEPS in the 'hooks' array just before the 'gyp' hook.

      {
        'name': 'apply_custom_patch',
        'pattern': '.',
        'action': ['apply_issue',
                   '--root_dir', 'src/third_party/skia',
                   '--issue', '1873923002',
                   '--patchset', '160001',
                   '--server', 'https://codereview.chromium.org',
                   '--force',
                   '--ignore_deps',
                   '-v',
                   '-v',
                   '--no-auth',
                   '--blacklist', 'DEPS'
        ],
      },

Modify the 'issue' and 'patchset' to the appropriate values.
If this is for a project other than Skia, update the 'root_dir' and 'server'.
Note that this can be used multiple times to apply multiple issues.

To find the patchset number in Rietveld use the URL of the '[raw]' (old UI) or
'Raw Patch' (new UI) link on the desired patch. The last segment of this URL
has the form 'issue\<issue>_\<patchset>.diff'.

An example of this being used can be seen at
https://crrev.com/1877673002/#ps120001 .

To test locally, run `gclient runhooks` to update the Skia source code.
Note that if your local skia patch in `third_party/skia` isn't clean (e.g., you
already applied some patch to it), then `gclient runhooks` won't successfully
run. In that case, run `git reset --hard` inside `third_party/skia` before
`gclient runhooks`.

Arbitrary changes
-----------------
If the patch is to files where the above is not possible, then it is still
possible to patch the files manually by adding the following to
\<chromium>/src/DEPS in the 'hooks' array just before the 'gyp' hook.

      {
        'name': 'apply_custom_patch',
        'pattern': '.',
        'action': ['python',
                   '-c', 'from distutils.dir_util import copy_tree; copy_tree("src/patch/", "src/");'
        ],
      },

Then, copy all 'out of tree' files into \<chromium>/src/patch/, using the same
directory structure used by Chromium. When 'gclient runhooks' is run, the files
in \<chromium>/src/patch/ will be copied to and overwrite corresponding files in
\<chromium>/src/. For example, if changing \<skia>/include/core/SkPath.h, place
a copy of the modified SkPath.h at
\<chromium>/src/patch/third_party/skia/include/core/SkPath.h.

An example of this being used can be seen at
https://crrev.com/1866773002/#ps20001 .


Try the patch
-------------
After committing a \<chromium>/src/DEPS or \<chromium>/src/patch/ change
locally, 'git cl upload' can be used in the usual way. Be sure to add
'COMMIT=false' to the issue description to avoid accidentally checking it in.
