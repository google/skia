Milestone Schedule
==================

On a six week cadence aligned with Chromium, Skia cuts milestone branches.
Clients wishing to stay on a relatively stable level of Skia often utilize these
branches.

On the branch date, a healthy level of Skia near HEAD is chosen.  After this
point, for the next six weeks, only high priority fixes are checked into the branch.
After the six week period, when another branch is cut, only critical (typically
security) fixes will be committed to any previous branch.

Skia 2016 schedule:

  Milestone | Branch Date (end of day)
  ----------|-------------------------
  50        | 02/25/16
  51        | 04/07/16
  52        | 05/19/16
  53        | 06/30/16
  54        | 08/25/16
  55        | 10/06/16
  56        | 11/17/16

The current milestone is included in the headers in
[SkMilestone.h](https://skia.googlesource.com/skia/+/master/include/core/SkMilestone.h).
