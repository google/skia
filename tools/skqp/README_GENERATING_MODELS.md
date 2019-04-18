How SkQP Generates Render Test Models
=====================================

We will, at regular intervals, generate new models from the [master branch of
Skia][1].  Here is how that process works:

0.  Choose a commit to make the branch from

        COMMIT=origin/master

    Or use the script to find the best one:

        cd SKIA_SOURCE_DIRECTORY
        git fetch origin
        COMMIT=$(python tools/skqp/find_commit_with_best_gold_results.py \
                 origin/master ^origin/skqp/dev)

1.  Get the positively triaged results from Gold and generate models:

        cd SKIA_SOURCE_DIRECTORY
        git fetch origin
        git checkout "$COMMIT"
        python tools/skqp/cut_release.py HEAD~10 HEAD

    This will create the following files:

        platform_tools/android/apps/skqp/src/main/assets/files.checksum
        platform_tools/android/apps/skqp/src/main/assets/skqp/rendertests.txt
        platform_tools/android/apps/skqp/src/main/assets/skqp/unittests.txt

    These three files can be commited to Skia to create a new commit.  Make
    `origin/skqp/dev` a parent of this commit (without merging it in), and
    push this new commit to `origin/skqp/dev`, using this script:

        sh tools/skqp/branch_skqp_dev.sh

    Review and submit the change:

        git push origin HEAD:refs/for/skqp/dev
        bin/sysopen https://review.skia.org/$(bin/gerrit-number HEAD)

    (Optional) Make a SkQP APK.

        tools/skqp/docker_build_universal_apk.sh

    (Optional) Test the SkQP APK:

        tools/skqp/test_apk.sh (LOCATION)/skqp-universal-debug.apk

    (Once changes land) Upload the SkQP APK.

        tools/skqp/upload_apk HEAD (LOCATION)/skqp-universal-debug.apk


`tools/skqp/cut_release.py`
---------------------------

This tool will call `make_skqp_model` to generate the `m{ax,in}.png` files for
each render test.

Then it calls `jitter_gms` to see which render tests pass the jitter test.
`jitter_gms` respects the `bad_gms.txt` file by ignoring the render tests
enumerated in that file.  Tests which pass the jitter test are enumerated in
the file `good.txt`, those that fail in the `bad.txt` file.

Next, the `skqp/rendertests.txt` file is created.  This file lists the render
tests that will be executed by SkQP.  These are the union of the tests
enumerated in the `good.txt` and `bad.txt` files.  If the render test is found
in the `good.txt` file and the model exists, its per-test threshold is set
to 0 (a later CL can manually change this, if needed).  Otherwise, the
threshold is set to -1; this indicated that the rendertest will be executed (to
verify that the driver will not crash), but the output will not be compared
against the model.  Unnecessary models will be removed.

Next, all of the files that represent the models are uploaded to cloud storage.
A single checksum hash is kept in the  `files.checksum` file.  This is enough
to re-download those files later, but we don't have to fill the git repository
with a lot of binary data.

Finally, a list of the current gpu unit tests is created and stored in
`skqp/unittests.txt`.

[1]: https://skia.googlesource.com/skia/+log/master "Skia Master Branch"
[2]: https://gold.skia.org/search                   "Skia Gold Search"
