Milestone Schedule
==================

On a cadence aligned with Chromium, Skia cuts milestone branches.
Chromium has
[announced](https://blog.chromium.org/2021/03/speeding-up-release-cycle.html) a
plan to move to 4 week milestones, starting with Chrome 94, and Skia will align.

Clients wishing to stay on a relatively stable level of Skia often utilize these
branches.

On the branch date, a healthy level of Skia near HEAD is chosen.  After this
point, for the next six weeks, only high priority fixes are checked into the branch.
After the six week period, when another branch is cut, only critical (typically
security) fixes will be committed to any previous branch.

Skia 2020 schedule:

  Milestone | Branch Date (beginning of day)
  ----------|-------------------------------
  81        | 01/30/20
  82        | 03/12/20
  83        | 04/02/20*
  84        | 05/14/20
  85        | 06/25/20
  86        | 08/20/20
  87        | 10/01/20
  88        | 11/12/20

Note that 82 was abandoned by Chromium and 83 schedule moved in due to COVID-19 impact.
Skia is maintaining 82 for our customers. Future dates are subject to change.

Skia 2021 schedule:

  Milestone | Branch Date (beginning of day)
  ----------|-------------------------------
  89        | 01/14/21
  90        | 02/25/21
  91        | 04/08/21
  92        | 05/20/21
  93        | 07/15/21
  94        | 08/12/21
  95        | 09/09/21
  96        | 10/07/21
  97        | 11/04/21
  98        | 12/09/21


The current milestone is included in the headers in
[SkMilestone.h](https://skia.googlesource.com/skia/+/master/include/core/SkMilestone.h).
