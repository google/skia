Milestone Schedule
==================

On a six week cadence aligned with Chromium, Skia cuts milestone branches.
Clients wishing to stay on a relatively stable level of Skia often utilize these
branches.

On the branch date, a healthy level of Skia near HEAD is chosen.  After this
point, for the next six weeks, only high priority fixes are checked into the branch.
After the six week period, when another branch is cut, only critical (typically
security) fixes will be committed to any previous branch.

Skia 2019 schedule:

  Milestone | Branch Date (beginning of day)
  ----------|-------------------------------
  73        | 01/24/19
  74        | 03/07/19
  75        | 04/18/19
  76        | 05/30/19
  77        | 07/25/19
  78        | 09/05/19
  79        | 10/17/19
  80        | 12/05/19

The current milestone is included in the headers in
[SkMilestone.h](https://skia.googlesource.com/skia/+/master/include/core/SkMilestone.h).
