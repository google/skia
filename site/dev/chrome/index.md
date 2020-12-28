Skia in Chrome
==============

Changes to the Skia repository will be rolled into Chromium by the AutoRoll bot
several times per day.

If you have a Skia change that needs to be tested in Chrome, or which requires
associated changes in that repository, see the guides in this section for tips
on execution.

For problems in Chromium related to Skia rolls:

  * Go to https://autoroll.skia.org/r/skia-autoroll. Login with google.com
    account and click the STOP button to pause new rolls.
  * Revert the offending DEPS roll.
  * If an obvious owner cannot be found in the list of CLs, assign to the Skia
    Gardener, listed in the gardeners widget on https://status.skia.org and as
    a reviewer on the roll CL.
  * If the Skia Gardener cannot be assigned, cc them and assign the issue to hcm@.

For more tips on bug triage and labeling, see the [Issue Tracker page](../../user/issue-tracker/).

Branching for Chrome
--------------------

Every 6 weeks, we cut a new branch in Skia to reflect the new release branch in
Chrome, eg. [refs/heads/chrome/m75](https://skia.googlesource.com/skia/+/chrome/m75).
This process is simplified by running [tools/chrome_release_branch](https://skia.googlesource.com/skia/+/7a5b6ec0f6c01d3039e3ec30de6f8065ffc8aac4/tools/chrome_release_branch.py').
This script handles creation of the branch itself, as well as associated
housekeeping like updating the Chrome milestone number for the next release,
setting up the [commit queue]('https://skia.googlesource.com/skia/+/infra/config/commit-queue.cfg')
for the new branch. For example:

    tools/chrome_release_branch <commit hash>

