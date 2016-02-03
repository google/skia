Tree Sheriffs Documentation
===========================

### Contents ###

*   [What does a sheriff do?](#what_is_a_sheriff)
    +   [Skia tree](#skia_tree)
    +   [DEPS rolls](#deps_rolls)
    +   [Gold and Perf](#gold_and_perf)
    +   [Documentation](#sheriff_doc)
*   [View current and upcoming sheriffs](#view_current_upcoming_sheriffs)
*   [How to swap sheriff shifts](#how_to_swap)
*   [Tips for sheriffs](#tips)
    +   [When to file bugs](#when_to_file_bugs)
    +   [How to close or re-open the tree](#how_close_tree)
    +   [Compile bot failures automatically close the tree](#tree_closers)
    +   [How to revert a CL](#how_to_revert)
    +   [What to do if DEPS roll fails to land](#deps_roll_failures)
    +   [How to rebaseline](#how_to_rebaseline)


<a name="what_is_a_sheriff"></a>
What does a sheriff do?
-----------------------

A sheriff keeps an eye on the tree, DEPS rolls, Gold tool and the Perf tool.

Below is a brief summary of what the sheriff does for each task:

<a name="skia_tree"></a>
### Skia tree
* Understand the [buildbots infrastructure](https://skia.org/dev/testing/buildbot).
* Start watching the [status page](https://status.skia.org) for bot breakages.
* Track down people responsible for breakages and revert broken changes if there is no easy fix.
* Close and open the [tree](http://skia-tree-status.appspot.com).
* Keep the builder comments on the [status page](https://status.skia.org) up to date.
* File or follow up with [BreakingTheBuildbots bugs](https://bug.skia.org/?q=label:BreakingTheBuildbots). See the tip on [when to file bugs](#when_to_file_bugs).

<a name="deps_rolls"></a>
### DEPS rolls
* Ensure that [AutoRoll Bot](https://skia-tree-status.appspot.com/set_arb_action)'s DEPS rolls land successfully.

<a name="gold_and_perf"></a>
### Gold and Perf
* Pay attention for new [Perf](https://perf.skia.org/) and [Gold](https://gold.skia.org/) alerts (by clicking on the bell at the top right of the [status page](https://status.skia.org)).
* The sheriff's duty here is to make sure that when developers introduce new images or new perf regressions, that they are aware of what happened, and they use these tools to take appropriate action.
 
<a name="sheriff_doc"></a>
### Documentation
* Improve/update this documentation page for future sheriffs, especially the [Tips section](#tips).

In general, sheriffs should have a strong bias towards actions that keep the tree green and then open; if a simple revert can fix the problem, the sheriff <b>should revert first and ask questions later</b>.


<a name="view_current_upcoming_sheriffs"></a>
View current and upcoming sheriffs
----------------------------------

The list of sheriffs is specified in the [skia-tree-status web app](https://skia-tree-status.appspot.com/sheriff). The current sheriff is highlighted in green.
The banner on the top of the [status page](https://status.skia.org) also displays the current sheriff.


<a name="how_to_swap"></a>
How to swap sheriff shifts
--------------------------

If you need to swap shifts with someone (because you are out sick or on vacation), please get approval from the person you want to swap with. Then send an email to skiabot@google.com to have someone make the database change (or directly ping rmistry@).


<a name="tips"></a>
Tips for sheriffs
-----------------

<a name="when_to_file_bugs"></a>
### When to file bugs

Pay close attention to the "Failures" view in the [status page](https://status.skia.org).
Look at all existing [BreakingTheBuildbots bugs](https://bug.skia.org/?q=label:BreakingTheBuildbots). If the list is kept up to date then it should accurately represent everything that is causing failures. If it does not, then please file/update bugs accordingly.


<a name="how_close_tree"></a>
### How to close or re-open the tree

1. Go to [skia-tree-status.appspot.com](skia-tree-status.appspot.com).
2. Change the status.
 *  To close the tree, include the word "closed" in the status.
 * To open the tree, include the word "open" in the status.
 * To caution the tree, include the word "caution" in the status.


<a name="how_to_submit_when_tree_closed"></a>
### How to submit when the tree is closed

* Submit manually using the "git cl land" with the --bypass-hooks flag.
* Add "NOTREECHECKS=true" to your CL description and use the CQ as usual.


<a name="tree_closers"></a>
### Compile bot failures automatically close the tree

A failure of the build steps in all compile bots automatically closes the tree. Sheriffs will have to manually reopen the tree manually when they deem the problem fixed.

Note: The tree is not closed automatically if the last run of the failed compile builder had the same failing step. The tree is also not closed if the tree was automatically closed less than 10 mins ago. If the tree is already closed then no action is taken.


<a name="how_to_revert"></a>
### How to revert a CL

See the revert documentation [here](https://skia.org/dev/contrib/revert).


<a name="deps_roll_failures"></a>
### What to do if DEPS roll fails to land

A common cause of DEPS roll failures are layout tests. Find the offending Skia CL by examining the commit hash range in the DEPS roll and revert (or talk to the commit author if they are available). If you do revert then keep an eye on the next DEPS roll to make sure it succeeds.

If a Skia CL changes layout tests, but the new images look good, the tests need to be rebaselined. See [Rebaseline Layout Tests](#how_to_rebaseline).

<a name="how_to_rebaseline"></a>
### Rebaseline Layout Tests (i.e., add suppressions)

* First create a Chromium bug:
  * goto [crbug.com](https://crbug.com)
  * Make sure you’re logged in with your Chromium credentials
  * Click “New Issue”
  * Summary: “Skia image rebaseline”
  * Description:
      * DEPS roll #,
      * Helpful message about what went wrong (e.g., “Changes to how lighting is scaled in Skia r#### changed the following images:”)
      * Layout tests effected
      * You should copy the list of affected from stdio of the failing bot
  * Status: Assigned
  * Owner: yourself
  * cc: reed@, bsalomon@, robertphillips@ & developer responsible for changes
  * Labels: OS-All & Cr-Blink-LayoutTests
  * If it is filter related, cc senorblanco@

* (Dispreferred but faster) Edit [skia/skia_test_expectations.txt](https://chromium.googlesource.com/chromium/src/+/master/skia/skia_test_expectations.txt)
  * Add # comment about what has changed (I usually paraphrase the crbug text)
  * Add line(s) like the following after the comment:
      * crbug.com/<bug#youjustcreated> foo/bar/test-name.html [ ImageOnlyFailure ]
  * Note: this change is usually done in the DEPS roll patch itself

* (Preferred but slower) Make a separate Blink patch by editing LayoutTests/TestExpectations
  * Add # comment about what has changed (I usually paraphrase the crbug text)
  * Add line(s) like the following after the comment:
      * crbug.com/<bug#youjustcreated> foo/bar/test-name.html [ NeedsManualRebaseline ]
  * Commit the patch you created and wait until it lands and rolls into Chrome

* Retry the DEPS roll (for the 1st/dispreferred option this usually means just retrying the layout bots)
* Make a Blink patch by editing LayoutTests/TestExpectations
  * Add # comment about what has changed 
  * Add line(s) like the following after the comment:
      * crbug.com/<bug#youjustcreated> foo/bar/test-name.html [ NeedsRebaseline ]
        * (if you took the second option above you can just edit the existing line(s))

* If you took the first/dispreferred option above:
  * Wait for the Blink patch to roll into Chrome
  * Create a Chrome patch that removes your suppressions from skia/skia_test_expectations.txt


