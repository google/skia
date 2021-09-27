---
title: 'Android Gardener Documentation'
linkTitle: 'Android Gardener Documentation'
---

### Contents

- [What does a Android Gardener do?](#what_is_a_android_gardener)
- [Android Autorollers](#autoroller_doc)
- [View current and upcoming rotations](#view_current_upcoming_rotations)
- [How to swap rotation shifts](#how_to_swap)

<a name="what_is_a_android_gardener"></a> What does a Android Gardener do?

---

The Android Gardener has two primary jobs:

1. Monitor and approve the semi-autonomous
   [git merges](https://googleplex-android-review.git.corp.google.com/#/q/owner:31977622648%2540project.gserviceaccount.com+status:open)
   from Skia's repository into the Android source tree. See autoroller
   documentation <a href="#autoroller_doc">here</a> for details on how to
   interact with it.

2. Stay on top of incoming Android-related bugs in both the
   [Skia](https://bugs.chromium.org/p/skia/issues/list?can=2&q=OpSys%3DAndroid&sort=-id&colspec=ID+Type+Status+Priority+Owner+Summary&cells=tiles)
   and
   [Android](https://buganizer.corp.google.com/issues?q=assignee:skia-android-triage%20status:open)
   bug trackers. For Skia bugs, this means triaging and assigning all Android
   bugs that are currently unassigned. For Android, this means following the
   [Android guidelines](http://go/android-buganizer) to verifying that all Skia
   bugs are TL-triaged (if not reach out to djsollen@).

The Android Gardener's job is NOT to address issues in Perf and Gold. You'll get
your chance when you are the general Skia Gardener.

<a name="autoroller_doc"></a> Android Autorollers

---

The Android autoroller into the master branch runs on
[https://skia-autoroll.corp.goog/r/android-master-autoroll](https://skia-autoroll.corp.goog/r/android-master-autoroll) and
is accessible only to Googlers.<br/> The autoroller's status is displayed on
Skia's [status page](https://status.skia.org/).

You can send the autoroller into dry run mode via the UI. The uploaded change
will not autosubmit when it is in dry run mode.

You can also stop the autoroller via the UI. This is useful in cases where a
failure needs to be investigated and you do not want to waste TH resources by
running unnecessary tests.

If the autoroller displays an error in the UI then look for more detail in it's
[cloud logs](https://pantheon.corp.google.com/logs/viewer?project=google.com:skia-buildbots&resource=logging_log%2Fname%2Fandroid-master-autoroll&logName=projects%2Fgoogle.com:skia-buildbots%2Flogs%2Fautoroll).

If you need any more information about the autoroller please look at
[skia:5538](https://bugs.chromium.org/p/skia/issues/detail?id=5538) or ask
rmistry@ / skiabot@.

We also have autorollers into release branches (also restricted only to
Googlers):

- [https://android-o-roll.skia.org](https://android-o-roll.skia.org)
  ([cloud logs](https://pantheon.corp.google.com/logs/viewer?project=google.com:skia-buildbots&resource=logging_log%2Fname%2Fandroid-o-autoroll&logName=projects%2Fgoogle.com:skia-buildbots%2Flogs%2Fautoroll)).

Changes created by these rollers need to be manually approved.<br/> The changes
created by the release rollers:

- Include all authors of merged changes so that they can watch the roll.
- Extracts all buganizer bugs of the form 'BUG=b/123' or 'Bug: b/456' and
  creates a single line in the merge change 'Bug: 123, 456'.
- Collects all 'Test: ' lines and carries them over to the merge change.

<a name="view_current_upcoming_rotations"></a> View current and upcoming
rotations

---

The list of Android Gardeners is specified
[here](https://rotations.corp.google.com/rotation/5296436538245120). The
gardeners widget on the [status page](https://status.skia.org) also displays the
current gardeners.

<a name="how_to_swap"></a> How to swap rotation shifts

---

If you need to swap shifts with someone (because you are out sick or on
vacation), please get approval from the person you want to swap with and
directly make the swap via the
[rotations page](https://rotations.corp.google.com/rotation/5296436538245120).
