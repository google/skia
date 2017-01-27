Using Gerrit without git-cl
===========================

setup
-----

    cd ...skia_source_dir...

    curl -Lo .git/hooks/commit-msg \
      https://skia-review.googlesource.com/tools/hooks/commit-msg

    chmod +x .git/hooks/commit-msg

    git remote set-url origin https://skia.googlesource.com/skia.git

    git config branch.autosetuprebase always


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

        MSG="$(git log --format='%B' ^@{u} @)"
        git reset --soft $(git merge-base @ @{u})
        git commit -m "$MSG" -e

4.  Push to Gerrit

        git push origin @:refs/for/master%cc=reviews@skia.org


updating a change
-----------------


1.  Edit your commits more.

        echo 3 > whitespace.txt
        git commit -a --amend --reuse-message=@

2.  Re-squash if needed.


3.  Push to Gerrit

        git push origin @:refs/for/master%m=this_is_a_message

