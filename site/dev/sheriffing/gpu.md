GPU Wrangler Documentation
===========================

### Contents ###

*   [What does a GPU wrangler do?](#what_is_a_wrangler)
*   [Tracking Wrangler Work do?](#tracking)
*   [View current and upcoming wranglers](#view_current_upcoming_wranglers)
*   [How to swap wrangler shifts](#how_to_swap)
*   [Tips for wranglers](#tips)


<a name="what_is_a_wrangler"></a>
What does a  GPU Wrangler do?
-----------------------

The wrangler has three main jobs:

1) Stay on top of incoming GPU-related bugs from clients in various bug trackers. This means triaging and assigning bugs that have a clear owner and investigating and possibly fixing bugs that don't.


2) Improve the reliability of the GPU bots. This includes dealing with flaky images, crashing bots, etc. We have a never ending set of machine or driver specific issues to deal with. We often brush them under the rug so that we have time for the "real work." When you're wrangler this is "real work."


3) Improve our tooling. This includes writing new tools and improving existing test tools. Expected results are faster bot run times, more accurate testing, faster testing, surfacing new useful data, and improving debuggability.


The wrangler should always prioritize dealing with incoming bugs. The balance of a wrangler's time should be spent divided as seen fit between 2) and 3). It is expected that as much as possible a wrangler puts normal work on pause and focuses on wrangler tasks for the full week. It is ok (and encouraged) to take a deep dive on one particular facet of the wrangler duties and drive it as far as possible during wrangler week (while staying on top of incoming bugs).

Note that the wrangler's job is NOT to spend an abnormal amount of time triaging images, filing bugs for failing bots, or shepherding DEPS rolls. You'll get your chance when you are the general Skia sheriff.

<a name="tracking"></a>
Tracking Wrangler Work
-----------------------
Outside of bug reports, a wrangler should track their progress so that a future wrangler can pick up any batons left shy of the finish line at week's end. This doc is the written history of wrangling:
https://goto.google.com/skgpu-wrangler-notes

Also, whenever a wrangler figures out how to accomplish a wrangly task (e.g. run a set of Chromium tests that aren't well documented or a cool OpenGL trick used to debug a gnarly issue) the tips section of this doc should be updated to assist future wranglers.


<a name="view_current_upcoming_wranglers"></a>
View current and upcoming wranglers
----------------------------------

The list of wranglers is specified in the [tree-status web app](http://tree-status.skia.org/wrangler). The current wrangler is highlighted in green.
The banner on the top of the [status page](https://status.skia.org) also displays the current wrangler.


<a name="how_to_swap"></a>
How to swap wrangler shifts
--------------------------

If you need to swap shifts with someone (because you are out sick or on vacation), please get approval from the person you want to swap with. Then send an email to skia-gpu-team@google.com and cc rmistry@.


<a name="tips"></a>
Tips for Wrangers
-----------------

Please see [this](https://docs.google.com/a/google.com/document/d/1Q1A5T5js4MdqvD0EKjCgNbUBJfRBMPKR3OZAkc-2Tvc/edit?usp=sharing) doc.
