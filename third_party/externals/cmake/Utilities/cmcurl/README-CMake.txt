The Utilities/cmcurl directory contains a reduced distribution
of the curl source tree with only the library source code and
CMake build system.  It is not a submodule; the actual content is part
of our source tree and changes can be made and committed directly.

We update from upstream using Git's "subtree" merge strategy.  A
special branch contains commits of upstream curl snapshots and
nothing else.  No Git ref points explicitly to the head of this
branch, but it is merged into our history.

Update curl from upstream as follows.  Create a local branch to
explicitly reference the upstream snapshot branch head:

 git branch curl-upstream 3fe5d9bf

Use a temporary directory to checkout the branch:

 mkdir curl-tmp
 cd curl-tmp
 git init
 git pull .. curl-upstream
 rm -rf *

Now place the (reduced) curl content in this directory.  See
instructions shown by

 git log 3fe5d9bf

for help extracting the content from the upstream repo.  Then run
the following commands to commit the new version.  Substitute the
appropriate date and version number:

 git add --all

 GIT_AUTHOR_NAME='Curl Upstream' \
 GIT_AUTHOR_EMAIL='curl-library@cool.haxx.se' \
 GIT_AUTHOR_DATE='Wed Sep 10 08:07:58 2014 +0200' \
 git commit -m 'curl 7.38.0 (reduced)' &&
 git commit --amend

Edit the commit message to describe the procedure used to obtain the
content.  Then push the changes back up to the main local repository:

 git push .. HEAD:curl-upstream
 cd ..
 rm -rf curl-tmp

Create a topic in the main repository on which to perform the update:

 git checkout -b update-curl master

Merge the curl-upstream branch as a subtree:

 git merge -s recursive -X subtree=Utilities/cmcurl \
           curl-upstream

If there are conflicts, resolve them and commit.  Build and test the
tree.  Commit any additional changes needed to succeed.

Finally, run

 git rev-parse --short=8 curl-upstream

to get the commit from which the curl-upstream branch must be started
on the next update.  Edit the "git branch curl-upstream" line above to
record it, and commit this file.
