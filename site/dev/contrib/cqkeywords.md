Commit Queue Keywords
=====================

See [CQ
documentation](https://chromium.googlesource.com/chromium/src/+/master/docs/infra/cq.md)
for more information.

Options in the form "Key: Value"  must appear in the last paragraph of the
commit message to be used.


Commit
------

If you are working on experimental code and do not want to risk accidentally
submitting the change via the CQ, then you can mark it with "Commit: false".
The CQ will immediately abandon the change if it contains this option.
To do a dry run through the CQ please use Gerrit's [CQ Dry
Run](https://groups.google.com/a/chromium.org/forum/#!topic/chromium-dev/G5-X0_tfmok)
feature.

    Commit: false

The CQ will run through its list of verifiers (reviewer check, trybots, tree check,
presubmit check), and will close the issue instead of committing it.


No-Dependency-Checks
--------------------

    No-Dependency-Checks: true

The CQ rejects patchsets with open dependencies. An open dependency exists when a CL
depends on another CL that is not yet closed. You can skip this check with this keyword.


Cq-Include-Trybots
------------------

Allows you to add arbitrary trybots to the CQ's list of default trybots.
The CQ will block till these tryjobs pass just like the default list of tryjobs.

This is the format of the values of this keyword:

    Cq-Include-Trybots: bucket1:bot1,bot2;bucket2:bot3,bot4

Multiple lines are allowed:

    Cq-Include-Trybots: bucket1:bot1
    Cq-Include-Trybots: bucket1:bot2
    Cq-Include-Trybots: bucket2:bot3
    Cq-Include-Trybots: bucket2:bot4

Here are some real world examples:

    Cq-Include-Trybots: master.tryserver.chromium.linux:linux_chromium_asan_rel_ng
    Cq-Include-Trybots: skia.primary:Test-Win10-Clang-ShuttleC-GPU-GTX960-x86_64-Debug-All-ANGLE
    Cq-Include-Trybots: luci.skia.skia.primary:Build-Debian9-Clang-x86-devrel-Android_SKQP

    FIXME: what bucket are skia bots in now?


No-Tree-Checks
--------------

If you want to skip the tree status checks, to make the CQ commit a CL even if
the tree is closed, you can add the following line to the CL description:

    No-Tree-Checks: true

This is discouraged, since the tree is closed for a reason. However, in rare
cases this is acceptable, primarily to fix build breakages (i.e., your CL will
help in reopening the tree).


No-Presubmit
------------

If you want to skip the presubmit checks, add the following line to the CL description:

    No-Presubmit: true


No-Try
------

If you cannot wait for the try job results, you can add the following line to
the CL description:

    No-Try: true

The CQ will then not run any try jobs for your change and will commit the CL as
soon as the tree is open, assuming the presubmit check passes.
