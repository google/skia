Skia Gold
=========

Overview
--------

Gold is a web application that compares the images produced by our bots against
known baseline images.  
Key features:  

* Baselines are managed in Gold outside of Git, but in lockstep with Git commits.
* Each commit creates >500k images.
* Deviations from the baseline are triaged after a CL lands and images are
  triaged as either `positive` or `negative`.  'Positive' means the diff is
  considered acceptable.  'Negative' means the diff is considered unacceptable
  and requires a fix.
  If a CL causes Skia to break it is reverted or an additional CL is landed to
  fix the problem.
* We test across a range of dimensions, e.g.:

  - OS (Windows, Linux, Mac, Android, iOS)
  - Architectures (Intel, ARM)
  - Backends (CPU, OpenGL, Vulkan etc.)
  - etc.

* Written in Go, Polymer and deployed on the Google Cloud.  The code
  is in the [Skia Infra Repository](https://github.com/google/skia-buildbot).

Recommended Workflows
---------------------
### How to best use Gold for commonly faced problems ###

These instructions will refer to various views which are accessible via the left
navigation on [gold.skia.org](https://gold.skia.org/).  
View access is public, triage access is granted to
Skia contributors.  You must be logged in to triage.

Problem #1: As sheriff, I need to triage and “assign” many incoming new images.
-------------------------------------------------------------------------------
Solution today:

*   Access the By Blame view to see digests needing triage and associated
    owners/CLs
    +   Only untriaged digests will be shown by default
    +   Blame is not sorted in any particular order
    +   Digests are clustered by runs and the most minimal set of blame

<img src=BlameView.png style="margin-left:30px" align="left" width="800"/> <br clear="left">

*   Select digests for triage
    +   Digests will be listed in order with largest difference first
    +   Click to open the digest view with detailed information

<img src=Digests.png style="margin-left:40px" align="left" width="780"/> <br clear="left">

*   Open bugs for identified owner(s)
    +   The digest detail view has a link to open a bug from the UI
    +   Via the Gold UI or when manually entering a bug, copy the full URL of
        single digest into a bug report
    +   The URL reference to the digest in Issue Tracker will link the bug to
        the digest in Gold

<img src="IssueHighlight.png" style="margin-left:60px" align="left" width="720" border=1/> <br clear="left">

<br>

Future improvements:

*   Smarter, more granular blamelist

<br>

Problem #2: As a developer, I need to land a CL that may change many images.
----------------------------------------------------------------------------
To find your results:

*   Immediately following commit, access the By Blame view to find untriaged
    digest groupings associated with your ID
*   Click on one of the clusters including your CL to triage
*   Return to the By Blame view to walk through all untriaged digests involving
    your change
*   Note:  It is not yet implemented in the UI but possible to filter the view
    by CL.  Delete hashes in the URL to only include the hash for your CL.

<img src=BlameView.png style="margin-left:30px" align="left" width="800"/> <br clear="left">

To rebaseline images:

*   Access the Ignores view and create a new, short-interval (hours) ignore for
    the most affected configuration(s)

<img src=Ignores.png style="margin-left:30px" align="left" width="800"/> <br clear="left">


*   Click on the Ignore to bring up a search view filtered by the affected
    configuration(s)
*   Mark untriaged images as positive (or negative if appropriate)
*   Follow one of two options for handling former positives:
    +   Leave former positives as-is and let them fall off with time if there is
        low risk of recurrence
    +   Mark former positives as negative if needed to verify the change moving
        forward

Future improvements:

*   Trybot support prior to commit, with view limited to your CL  
*   Pre-triage prior to commit that will persist when the CL lands

<br>

Problem #3: As a developer or infrastructure engineer, I need to add a new or updated config.
---------------------------------------------------------------------------------------------
(ie: new bot, test mode, environment change)

Solution today:

*   Follow the process for rebaselining images:
    +   Wait for the bot/test/config to be committed and show up in the Gold UI
    +   Access the Ignores view and create a short-interval ignore for the
        configuration(s)
    +   Triage the ignores for that config to identify positive images
    +   Delete the ignore

Future improvements:

*   Introduction of a new or updated test can make use of try jobs and pre-triage.  
*   New configs may be able to use these features as well.

<br>

Problem #4: As a developer, I need to analyze the details of a particular image digest.
---------------------------------------------------------------------------------------
Solution:

*   Access the By Test view

<img src=ByTest.png style="margin-left:30px" align="left" width="800"/> <br clear="left">

*   Click the magnifier to filter by configuration
*   Access the Cluster view to see the distribution of digest results
    +   Use control-click to select and do a direct compare between data points
    +   Click on configurations under “parameters” to highlight data points and
        compare

<img src=ClusterConfig.png style="margin-left:30px" align="left" width="800"/> <br clear="left">

*   Access the Grid view to see NxN diffs

<img src=Grid.png style="margin-left:30px" align="left" width="800"/> <br clear="left">

*   Access the Dot diagram to see history of commits for the trace
    +   Each dot represents a commit
    +   Each line represents a configuration
    +   Dot colors distinguish between digests

<img src=DotDiagram.png style="margin-left:30px" align="left" width="800"/> <br clear="left">

<br>

Future improvements:

*   Large diff display of image vs image

<br>

Problem #5: As a developer, I need to find results for a particular configuration.
----------------------------------------------------------------------------------
Solution:

*   Access the Search view
*   Select any parameters desired to search across tests

<img src=Search.png style="margin-left:30px" align="left" width="800"/> <br clear="left">
