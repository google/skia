Using Gerrit without git-cl
===========================

setup
-----

    cd ...skia_source_dir...

    sh experimental/tools/setup-gerrit

Take a look at [the setup-gerrit script](../tools/setup-gerrit) for more detail.


creating a change
-----------------

1.  Create a topic branch

        git checkout -b TOPIC -t origin/master

2.  Make some commits.

        echo 1 > whitespace.txt
        git commit -a -m 'Change Foo'
        echo 2 > whitespace.txt
        git commit -a -m 'Change Foo again'

3.  Squash the commits:

        git squash-commits

    This is only needed if you have more than one commit on your branch.

4.  Push to Gerrit

        git gerrit-push-master


updating a change
-----------------


1.  Edit your commits more.

        echo 3 > whitespace.txt
        git amend-head

2.  Re-squash if needed.  (Not needed if you only amended your original commit.)


3.  Push to Gerrit.

        git gerrit-push-master this is a message

    The title of this patchset will be "this is a message".


