# How to Branch and Roll Chromium's ANGLE Dependency

ANGLE provides an implementation of OpenGL ES on Windows, which Chromium relies
upon for hardware accelerated rendering and WebGL support. Chromium specifies
its dependency on a specific version of ANGLE in the repository; this document
describes how to update that dependency, and, if necessary, create an ANGLE
branch to correspond to a branched release of Chrome.

## Rolling DEPS

Chromium's dependency on third-party projects is tracked in [the Chromium
repository's src/DEPS file]
(http://src.chromium.org/viewvc/chrome/trunk/src/DEPS). To update the ANGLE
dependency: * Find the line in this file that defines "src/third\_party/angle"
for deps (**not** deps\_os) * Change the [git SHA-1 revision number]
(http://git-scm.com/book/ch6-1.html) to be that of the commit on which Chromium
should depend. Please use the full SHA-1, not a shortened version. * You can
find the SHA-1 for a particular commit with `git log` on the appropriate branch
of the repository, or via [the public repository viewer]
(https://chromium.googlesource.com/angle/angle). * If using the public
repository viewer, you will need to select the branch whose log you wish to view
from the list on the left-hand side, and then click on the "tree" link at the
top of the resulting page. Alternatively, you can navigate to
`https://chromium.googlesource.com/angle/angle/+/<branch name>/` -- including
the terminating forward slash. (e.g.
`https://chromium.googlesource.com/angle/angle/+/master/`)

## Branching ANGLE

Sometimes, individual changes to ANGLE are needed for a release of Chrome which
has already been branched. If this is the case, a branch of ANGLE should be
created to correspond to the Chrome release version, so that Chrome may
incorporate only these changes, and not everything that has been committed since
the version on which Chrome depended at branch time. **Please note: Only ANGLE
admins can create a new branch.** To create a branch of ANGLE for a branched
Chrome release: * Determine what the ANGLE dependency is for the Chrome release
by checking the DEPS file for that branch. * Check out this commit as a new
branch in your local repository. * e.g., for [the Chrome 34 release at
chrome/branches/1847]
(http://src.chromium.org/viewvc/chrome/branches/1847/src/DEPS), the ANGLE
version is 4df02c1ed5e97dd54576b06964b1da67ea30238e. To check this commit out
locally and create a new branch named 'mybranch' from this commit, use: `git
checkout -b mybranch 4df02c1ed5e97dd54576b06964b1da67ea30238e
` * To create this new branch in the public repository, you'll need to push the
branch to the special Gerrit reference location, 'refs/heads/<branch name>'. You
must be an ANGLE administrator to be able to push this new branch. * e.g., to
use your local 'mybranch' to create a branch in the public repository called
'chrome\_m34', use: `git push origin mybranch:refs/heads/chrome_m34
` * The naming convention that ANGLE uses for its release-dedicated branches is
'chrome\_m##'.
