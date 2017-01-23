Android RoboCop Documentation
===========================

### Contents ###

*   [What does a Android RoboCop do?](#what_is_a_robocop)
*   [Android Autoroller](#autoroller_doc)
*   [View current and upcoming RoboCops](#view_current_upcoming_robocops)
*   [How to swap RoboCop shifts](#how_to_swap)


<a name="what_is_a_robocop"></a>
What does a Android RoboCop do?
-----------------------

The RoboCop has two primary jobs:

1) Monitor and approve the semi-autonomous [git merges](https://googleplex-android-review.git.corp.google.com/#/q/owner:31977622648%2540project.gserviceaccount.com+status:open) from Skia's repository into the Android source tree. See autoroller documentation <a href="#autoroller_doc">here</a> for details on how to interact with it.

2) Stay on top of incoming Android-related bugs in both the [Skia](https://bugs.chromium.org/p/skia/issues/list?can=2&q=OpSys%3DAndroid&sort=-id&colspec=ID+Type+Status+Priority+Owner+Summary&cells=tiles) and [Android](https://buganizer.corp.google.com/issues?q=componentid:1346%20status:open) bug trackers.  For Skia bugs, this means triaging and assigning all Android bugs that are currently unassigned.  For Android, this means following the [Android guidelines](go/android-buganizer) to verifying that all Skia bugs are TL-triaged (if not reach out to djsollen@).

The RoboCop's job is NOT to address issues in Perf and Gold. You'll get your chance when you are the general Skia sheriff.


<a name="autoroller_doc"></a>
Android Autoroller
------------------

The Android autoroller runs on the [client.skia.internal](https://chromegw.corp.google.com/i/client.skia.internal/console) master using the [merge_into_android.py](https://chrome-internal.googlesource.com/chrome/tools/build_limited/scripts/slave/+/master/skia/merge_into_android.py) recipe.

If you need to stop the autoroller then do the following steps:

* echo stop > /tmp/action
* gsutil cp /tmp/action gs://skia-android-autoroller/action

To turn the autoroller back on:

* echo start > /tmp/action
* gsutil cp /tmp/action gs://skia-android-autoroller/action

If you need any more information about the autoroller please look at [skia:6065](https://bugs.chromium.org/p/skia/issues/detail?id=6065) or ask rmistry@ / skiabot@.

<a name="view_current_upcoming_robocops"></a>
View current and upcoming RoboCops
----------------------------------

The list of RoboCops is specified in the [skia-tree-status web app](http://skia-tree-status.appspot.com/robocop). The current RoboCop is highlighted in green.
The banner on the top of the [status page](https://status.skia.org) also displays the current RoboCop.


<a name="how_to_swap"></a>
How to swap RoboCop shifts
--------------------------

If you need to swap shifts with someone (because you are out sick or on vacation), please get approval from the person you want to swap with. Then send an email to skia-android@google.com and cc rmistry@.
