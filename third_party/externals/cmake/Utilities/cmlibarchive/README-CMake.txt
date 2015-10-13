The Utilities/cmlibarchive directory contains a reduced distribution
of the libarchive source tree with only the library source code and
CMake build system.  It is not a submodule; the actual content is part
of our source tree and changes can be made and committed directly.

We update from upstream using Git's "subtree" merge strategy.  A
special branch contains commits of upstream libarchive snapshots and
nothing else.  No Git ref points explicitly to the head of this
branch, but it is merged into our history.

Update libarchive from upstream as follows.  Create a local branch to
explicitly reference the upstream snapshot branch head:

 git branch libarchive-upstream 37f225b7

Use a temporary directory to checkout the branch:

 mkdir libarchive-tmp
 cd libarchive-tmp
 git init
 git pull .. libarchive-upstream
 rm -rf *

Now place the (reduced) libarchive content in this directory.  See
instructions shown by

 git log 37f225b7

for help extracting the content from the upstream svn repo.  Then run
the following commands to commit the new version.  Substitute the
appropriate date and version number:

 git add --all

 GIT_AUTHOR_NAME='LibArchive Upstream' \
 GIT_AUTHOR_EMAIL='libarchive-discuss@googlegroups.com' \
 GIT_AUTHOR_DATE='Mon Apr 14 19:19:05 2014 -0700' \
 git commit -m 'libarchive 3.1.2-246-ga5a5d28b (reduced)' &&
 git commit --amend

Edit the commit message to describe the procedure used to obtain the
content.  Then push the changes back up to the main local repository:

 git push .. HEAD:libarchive-upstream
 cd ..
 rm -rf libarchive-tmp

Create a topic in the main repository on which to perform the update:

 git checkout -b update-libarchive master

Merge the libarchive-upstream branch as a subtree:

 git merge -s recursive -X subtree=Utilities/cmlibarchive \
           libarchive-upstream

If there are conflicts, resolve them and commit.  Build and test the
tree.  Commit any additional changes needed to succeed.

Finally, run

 git rev-parse --short=8 libarchive-upstream

to get the commit from which the libarchive-upstream branch must be started
on the next update.  Edit the "git branch libarchive-upstream" line above to
record it, and commit this file.
