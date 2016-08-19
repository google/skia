Issue Tracking
==============

The Skia Issue Tracker
----------------------
[Skia's Issue Tracker](https://bugs.chromium.org/p/skia/issues/list)
(bug.skia.org or skbug.com) is the primary bug database where we track all defect
reports and feature requests.

When filing a new issue, please select the appropriate template, most likely
"Defect report from user" or "Feature request".  Include an example
[fiddle](https://fiddle.skia.org) or image.  All issues will be triaged by our
program manager and assigned to the appropriate functional team.


Skia issues in the Chromium Tracker
-----------------------------------
Skia bugs found in Chrome may be filed in the [Chromium Tracker](https://bugs.chromium.org/p/chromium/issues/list) (crbug.com).

### Triage for Chromium developers
  * To have an issue triaged by the Skia team, add `Component:Internals>Skia`.
  * For problems related to Skia rolls where an obvious owner cannot be found in
    the list of CLs, assign to the Skia Sheriff, listed at the top of 
    [status.skia.org](https://status.skia.org) and as a reviewer on the roll CL.
    * If the Sheriff cannot be assigned, cc them and assign the issue to hcm@.
  * For GPU specific issues, add label `Hotlist-Ganesh`.
  * For image encoding or decoding issues, add
    `Component:Internals>Images>Codecs`.
