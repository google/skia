Commit Queue Keywords
=====================

COMMIT
------

If you are working on experimental code and do not want to risk accidentally
submitting the change via the CQ, then you can mark it with "COMMIT=false".
The CQ will immediately abandon the change if it contains this option.
To do a dry run through the CQ please use Rietveld's [dry run](https://groups.google.com/a/chromium.org/forum/#!topic/chromium-dev/G5-X0_tfmok) feature.

    COMMIT=false

The CQ will run through its list of verifiers (reviewer check, trybots, tree check,
presubmit check), and will close the issue instead of committing it.

NO_DEPENDENCY_CHECKS
--------------------

    NO_DEPENDENCY_CHECKS=true

The CQ rejects patchsets with open dependencies. An open dependency exists when a CL
depends on another CL that is not yet closed. You can skip this check with this keyword.

CQ_INCLUDE_TRYBOTS
------------------

Allows you to add arbitrary trybots to the CQ's list of default trybots.
The CQ will block till these tryjobs pass just like the default list of tryjobs.

This is the format of the values of this keyword:

    CQ_INCLUDE_TRYBOTS=master1:bot1,bot2;master2:bot3,bot4

Here are some real world examples:

    CQ_INCLUDE_TRYBOTS=tryserver.chromium:linux_layout_rel

    CQ_INCLUDE_TRYBOTS=tryserver.skia:Build-Mac10.9-Clang-x86_64-Debug

TBR
---

If you are a Skia committer and cannot wait for a review,
then you can include the TBR keyword in your CL's description.

Example:

    TBR=rmistry@google.com

NOTREECHECKS
------------

If you want to skip the tree status checks, to make the CQ commit a CL even if the tree is closed,
you can add the following line to the CL description:

    NOTREECHECKS=true

This is discouraged, since the tree is closed for a reason. However, in rare cases this is acceptable,
primarily to fix build breakages (i.e., your CL will help in reopening the tree).

NOPRESUBMIT
-----------

If you want to skip the presubmit checks, add the following line to the CL description:

    NOPRESUBMIT=true

NOTRY
-----

If you cannot wait for the try job results, you can add the following line to the CL description:

    NOTRY=true

The CQ will then not run any try jobs for your change and will commit the CL as soon as the tree is open, assuming the presubmit check passes.

NO_MERGE_BUILDS
---------------

This keyword prevents the Skia build masters from building this commit with others. Use it when your
commit may have effects that you don't want mis-attributed to other commits. Just include the keyword
somewhere in the commit message:

    NO_MERGE_BUILDS
