Skia Quickstart Guide
=====================


WARNING: Several steps in this guide are out of sync with our automatically-
tested, officially-supported processes for checking out and building Skia.
The officially supported processes are the ones documented in https://skia.org;
see https://skia.org/user/quick.

The steps documented within this file are more experimental in nature.


This guide assumes you've got `git`, `ninja`, and `python` on your path.

1. First, checkout Skia:
    * `git clone https://skia.googlesource.com/skia.git`
    * `cd skia`
2. Then download the dependencies.  You only need to rerun this when
   the dependencies change.
    * `python tools/git-sync-deps`
3. Create our Ninja build files from our Gyp meta-build files.  You only need
   to rerun this when you sync or change a `.gyp` file.
    * `GYP_GENERATORS=ninja ./gyp_skia`
4. Now, let's build Skia.  There are a few options:
    * `ninja -C out/Debug`: no optimization, asserts enabled
    * `ninja -C out/Release`: optimization, asserts disabled
    * `ninja -C out/Coverage`: no optimization, asserts enabled, code coverage generated
5. Run some tests:
    * `out/Debug/dm`: runs golden master tests from gm/, unit tests from tests/
6. Make some changes:
    * `git checkout -b my-new-feature origin/master`
    * `vim src/...`
    * `git commit -am "Changes for my new feature."`
    * `vim tests/...`
    * `git commit --amend -a`
    * `ninja -C out/Debug && out/Debug/dm && echo ok`
7. Rebase your change onto the latest Skia code:
    * `git pull --rebase`
    * `ninja -C out/Debug && out/Debug/dm && echo ok`
8. Upload your change and send it out for review:
    * `git cl upload -r my-skia-reviewer@google.com -s`
    * `git cl web`
9. Go through code review, get an LGTM, submit using the checkbox on the code review page.
