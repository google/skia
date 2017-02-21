Milestone Schedule
==================

On a six week cadence aligned with Chromium, Skia cuts milestone branches.
Clients wishing to stay on a relatively stable level of Skia often utilize these
branches.

On the branch date, a healthy level of Skia near HEAD is chosen.  After this
point, for the next six weeks, only high priority fixes are checked into the branch.
After the six week period, when another branch is cut, only critical (typically
security) fixes will be committed to any previous branch.

Skia 2017 schedule:

  Milestone | Branch Date (beginning of day)
  ----------|-------------------------------
  57        | 01/19/17
  58        | 03/02/17
  59        | 04/13/17
  60        | 05/25/17
  61        | 07/20/17
  62        | 08/31/17
  63        | 10/12/17
  64        | 11/30/17

The current milestone is included in the headers in
[SkMilestone.h](https://skia.googlesource.com/skia/+/master/include/core/SkMilestone.h).
