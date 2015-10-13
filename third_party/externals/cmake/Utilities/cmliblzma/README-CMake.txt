The Utilities/cmliblzma directory contains a reduced distribution
of the liblzma source tree with only the library source code and
CMake build system.  It is not a submodule; the actual content is part
of our source tree and changes can be made and committed directly.

We update from upstream using Git's "subtree" merge strategy.  A
special branch contains commits of upstream liblzma snapshots and
nothing else.  No Git ref points explicitly to the head of this
branch, but it is merged into our history.

Update liblzma from upstream as follows.  Create a local branch to
explicitly reference the upstream snapshot branch head:

 git branch liblzma-upstream c289e634

Use a temporary directory to checkout the branch:

 mkdir liblzma-tmp
 cd liblzma-tmp
 git init
 git pull .. liblzma-upstream
 rm -rf *

Now place the (reduced) liblzma content in this directory.  See
instructions shown by

 git log c289e634

for help extracting the content from the upstream svn repo.  Then run
the following commands to commit the new version.  Substitute the
appropriate date and version number:

 git add --all

 GIT_AUTHOR_NAME='liblzma upstream' \
 GIT_AUTHOR_EMAIL='xz-devel@tukaani.org' \
 GIT_AUTHOR_DATE='Sun Jun 30 19:55:49 2013 +0300' \
 git commit -m 'liblzma 5.0.5-gb69900ed (reduced)' &&
 git commit --amend

Edit the commit message to describe the procedure used to obtain the
content.  Then push the changes back up to the main local repository:

 git push .. HEAD:liblzma-upstream
 cd ..
 rm -rf liblzma-tmp

Create a topic in the main repository on which to perform the update:

 git checkout -b update-liblzma master

Merge the liblzma-upstream branch as a subtree:

 git merge -s recursive -X subtree=Utilities/cmliblzma \
           liblzma-upstream

If there are conflicts, resolve them and commit.  Build and test the
tree.  Commit any additional changes needed to succeed.

Finally, run

 git rev-parse --short=8 liblzma-upstream

to get the commit from which the liblzma-upstream branch must be started
on the next update.  Edit the "git branch liblzma-upstream" line above to
record it, and commit this file.
