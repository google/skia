Using Gerrit without git-cl
===========================

setup
-----

    cd ...skia_source_dir...

    sh experimental/tools/setup-gerrit

Take a look at [the setup-gerrit script](../tools/setup-gerrit) for more
detail.  You may only want some of what the script does.  All you really need
is the following:

    cd ...skia_source_dir...
    curl -Lo "$(git rev-parse --git-dir)/hooks/commit-msg"
      'https://gerrit-review.googlesource.com/tools/hooks/commit-msg'
    chmod +x "$(git rev-parse --git-dir)/hooks/commit-msg"
    git remote set-url origin 'https://skia.googlesource.com/skia.git'


creating a change
-----------------

1.  Create a topic branch

        git checkout -b TOPIC -t origin/master

2.  Make some commits.

        echo 1 > whitespace.txt
        git commit -a -m 'Change Foo'
        echo 2 > whitespace.txt
        git commit -a -m 'Change Foo again'

3.  If You have multiple commits in your branch, Gerrit will think you want
    multiple Changes that depend on each other.  If this is not what you want,
    you need to squash the commits, either with `git rebase --interactive` or
    with the `squash-commits` alias defined in `setup-gerrit`:

        git squash-commits

4.  Push to Gerrit

        git push origin @:refs/for/master

    If this is too much to remember, you can use the `gerrit-push-master` alias
    defined in `setup-gerrit`:

        git gerrit-push-master

    If you want to target a branch other than `master`, that can be specified here, too.
    For example:

        git push origin @:refs/for/chrome/m57


updating a change
-----------------


1.  Edit your commits more.

        echo 3 > whitespace.txt
        git commit -a --amend

    The `amend-head` alias defined in `setup-gerrit` may be easier to use,
    since it leaves the commit message untouched:

        git amend-head

2.  Re-squash if needed.  (Not needed if you only amended your original commit.)


3.  Push to Gerrit.

        git push origin @:refs/for/master

    If you want to set a comment message for this patch set, do this instead:

        git push origin @:refs/for/master%m=this_is_the_patch_set_comment_message

    The title of this patch set will be "this is the patch set comment message".

    Alternatively, `gerrit-push-master` uses its arguments to add a comment message:

        git gerrit-push-master this is the patch set comment message


