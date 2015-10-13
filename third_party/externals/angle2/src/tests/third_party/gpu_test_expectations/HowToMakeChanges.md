Because the ```gpu_test_expectations``` directory is based on parts of Chromium's ```gpu/config``
directory, we want to keep a patch of the changes added to make it compile with ANGLE. This
will allow us to merge Chromium changes easily in our ```gpu_test_expectations```.

In order to make a change to this directory, do the following:

 * copy the directory somewhere like in ```gpu_test_expectations_reverted```
 * in ```gpu_test_expectations_reverted``` run ```patch -p 1 -R < angle-mods.patch```
 * do your changes in ```gpu_test_expectations```
 * delete angle-mods.patch in both directories
 * run ```diff -rupN gpu_test_expectations_reverted gpu_test_expectations > angle-mods.patch```
 * copy ```angle-mods.patch``` in ```gpu_test_expectations```

How to update from Chromium:

 * ```git apply -R angle-mods.patch```, ```git add . -u```, ```git commit```
 * Copy over Chromium files, ```git add . -u```, ```git commit```
 * ```git revert HEAD~2```
 * ```rm angle-mods.patch```
 * ```git diff HEAD~1 (`)ls(`) > angle-mods.patch```,```git add angle-mods.patch```, ```git commit --amend```
 * ```git rebase -i``` to squash the three patches into one.

