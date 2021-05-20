
---
title: "Milestone Schedule"
linkTitle: "Milestone Schedule"

---


On a six week cadence aligned with Chromium, Skia cuts milestone branches.
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

The current milestone is included in the headers in
[SkMilestone.h](https://skia.googlesource.com/skia/+/main/include/core/SkMilestone.h).

