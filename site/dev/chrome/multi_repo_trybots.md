Multiple repo Chromium trybots
==============================

When a proposed Skia change will require a change in Chromium or Blink it is
often helpful to locally create the Chromium and Blink changes and test with the
proposed Skia change. This often happens with Skia API changes and changes
which affect Blink layout tests. While simple to do locally, this explains how
to do so on the Chromium trybots.

Skia only changes
-----------------
If the Skia patch is already in Gerrit and there are no associated Chromium
changes, then it is possible to just run the Chromium trybots. This will apply
the Skia patch and run the bot.

Skia and Chromium changes
-------------------------
If the Skia patch is already in Gerrit and there are associated Chromium
changes, then in the Chromium CL add the following to
\<chromium>/src/DEPS in the 'hooks' array.

      {
        'name': 'fetch_custom_patch',
        'pattern': '.',
        'action': [ 'git', '-C', 'src/third_party/skia/',
                    'fetch', 'https://skia.googlesource.com/skia', 'refs/changes/13/10513/13',
        ],
      },
      {
        'name': 'apply_custom_patch',
        'pattern': '.',
        'action': ['git', '-C', 'src/third_party/skia/',
                   '-c', 'user.name=Custom Patch', '-c', 'user.email=custompatch@example.com',
                   'cherry-pick', 'FETCH_HEAD',
        ],
      },

Modify the 'refs/changes/XX/YYYY/ZZ' to the appropriate values (where YYYY is
the numeric change number, ZZ is the patch set number and XX is the last two
digits of the numeric change number). This can be seen in the 'Download' link on
Gerrit.

If this is for a project other than Skia, update the checkout directory and
fetch source. Note that this can be used multiple times to apply multiple
issues.

An example of this being used can be seen at
https://crrev.com/2786433004/#ps1 .

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
        'action': ['python2',
                   '-c', 'from distutils.dir_util import copy_tree; copy_tree("src/patch/", "src/");'
        ],
      },

Then, copy all 'out of tree' files into \<chromium>/src/patch/, using the same
directory structure used by Chromium. When `gclient runhooks` is run, the files
in \<chromium>/src/patch/ will be copied to and overwrite corresponding files in
\<chromium>/src/. For example, if changing \<skia>/include/core/SkPath.h, place
a copy of the modified SkPath.h at
\<chromium>/src/patch/third_party/skia/include/core/SkPath.h.

An example of this being used can be seen at
https://crrev.com/1866773002/#ps20001 .


Try the patch
-------------
After committing a \<chromium>/src/DEPS or \<chromium>/src/patch/ change
locally, `git cl upload` can be used in the usual way. Be sure to add
`COMMIT=false` to the issue description to avoid accidentally checking it in.
