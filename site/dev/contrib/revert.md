How to revert a CL
==================

Using one-click revert
----------------------
*   Find the codereview issue for the CL you want to revert.
*   Click the "revert" button.

Using Git
---------

Update the local repository

    git fetch origin master

Create a local branch with origin/master as its start point.

    git checkout -b revert$RANDOM origin/master

Find the SHA1 of the commit you want to revert

    git log origin/master

Create a revert commit.

    git revert <SHA1>

Upload it to Gerrit.

    git cl upload

Land the revert in origin/master.

    git cl land

Delete the local revert branch.

    git checkout --detach && git branch -D @{-1}

