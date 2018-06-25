Milestone Schedule
==================

On a six week cadence aligned with Chromium, Skia cuts milestone branches.
Clients wishing to stay on a relatively stable level of Skia often utilize these
branches.

On the branch date, a healthy level of Skia near HEAD is chosen.  After this
point, for the next six weeks, only high priority fixes are checked into the branch.
After the six week period, when another branch is cut, only critical (typically
security) fixes will be committed to any previous branch.

Skia 2018 schedule:

  Milestone | Branch Date (beginning of day)
  ----------|-------------------------------
  65        | 01/18/18
  66        | 03/01/18
  67        | 04/12/18
  68        | 05/24/18
  69        | 07/19/18
  70        | 08/30/18
  71        | 10/11/18
  72        | 11/29/18

The current milestone is included in the headers in
[SkMilestone.h](https://skia.googlesource.com/skia/+/master/include/core/SkMilestone.h).
