Skia in Chrome
==============

Changes to the Skia repository will be rolled into Chromium (and Blink) by the
AutoRoll bot several times per day.

If you have a Skia change that needs to be tested in Chrome or Blink, or which
requires associated changes in those repositories, see the guides in this
section for tips on execution.

For problems in Chromium related to Skia rolls:

  * Go to https://autoroll.skia.org. Login with google.com account and click
    the STOP button to pause new rolls.
  * Revert the offending DEPS roll.
  * If an obvious owner cannot be found in the list of CLs, assign to the Skia
    Sheriff, listed at the top of https://status.skia.org and as a reviewer
    on the roll CL.
  * If the Sheriff cannot be assigned, cc them and assign the issue to hcm@. 

For more tips on bug triage and labeling, see the [Issue Tracker page](../../user/issue-tracker/).
