How to revert a CL
==================

Using one-click revert
----------------------
  * Find the codereview issue for the CL you want to revert.
  * Click the "revert" button.

Using Git
---------
    * git checkout master
    * git svn fetch && git svn rebase && gclient sync
    * git checkout -t -b <branch_name>
    * git log
    * <Find the SHA1 of the commit you want to revert>
    * git revert <SHA1>
    * git cl upload
    * git cl land
