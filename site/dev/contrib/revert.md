How to revert a CL
==================

Using one-click revert
----------------------
  * Find the codereview issue for the CL you want to revert.
  * Click the "revert" button.

Using Git
---------
    * git checkout master
    * git pull --rebase && gclient sync
    * git checkout -b <branch_name> origin/master
    * git log
    * <Find the SHA1 of the commit you want to revert>
    * git revert <SHA1>
    * git cl upload
    * git cl land
