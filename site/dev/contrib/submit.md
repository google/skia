How to submit a patch
=====================


Configure git
-------------

<!--?prettify lang=sh?-->

    git config --global user.name "Your Name"
    git config --global user.email you@example.com

Making changes
--------------

First create a branch for your changes:

<!--?prettify lang=sh?-->

    git config branch.autosetuprebase always
    git checkout -b my_feature origin/master

After making your changes, create a commit

<!--?prettify lang=sh?-->

    git add [file1] [file2] ...
    git commit

If your branch gets out of date, you will need to update it:

<!--?prettify lang=sh?-->

    git pull
    python tools/git-sync-deps

Adding a unit test
------------------

If you are willing to change Skia codebase, it's nice to add a test at the same
time. Skia has a simple unittest framework so you can add a case to it.

Test code is located under the 'tests' directory.

See [Writing Unit and Rendering Tests](../testing/tests) for details.

Unit tests are best, but if your change touches rendering and you can't think of
an automated way to verify the results, consider writing a GM test or a new page
of SampleApp. Also, if your change is the GPU code, you may not be able to write
it as part of the standard unit test suite, but there are GPU-specific testing
paths you can extend.

Submitting a patch
------------------

For your code to be accepted into the codebase, you must complete the
[Individual Contributor License
Agreement](http://code.google.com/legal/individual-cla-v1.0.html). You can do
this online, and it only takes a minute. If you are contributing on behalf of a
corporation, you must fill out the [Corporate Contributor License
Agreement](http://code.google.com/legal/corporate-cla-v1.0.html)
and send it to us as described on that page. Add your (or your organization's)
name and contact info to the AUTHORS file as a part of your CL.

Now that you've made a change and written a test for it, it's ready for the code
review! Submit a patch and getting it reviewed is fairly easy with depot tools.

Use git-cl, which comes with [depot
tools](http://sites.google.com/a/chromium.org/dev/developers/how-tos/install-depot-tools).
For help, run git-cl help.

### Find a reviewer

Ideally, the reviewer is someone who is familiar with the area of code you are
touching. If you have doubts, look at the git blame for the file to see who else
has been editing it.

### Uploading changes for review

Skia uses the Gerrit code review tool. Skia's instance is [skia-review](http://skia-review.googlesource.com).
Use git cl to upload your change:

<!--?prettify lang=sh?-->

    git cl upload

You may have to enter a Google Account username and password to authenticate
yourself to Gerrit. A free gmail account will do fine, or any
other type of Google account.  It does not have to match the email address you
configured using `git config --global user.email` above, but it can.

The command output should include a URL, similar to
(https://skia-review.googlesource.com/c/4559/), indicating where your changelist
can be reviewed.

### Request review

Go to the supplied URL or go to the code review page and select the **Your**
dropdown and click on **Changes**. Select the change you want to submit for
review and click **Reply**. Enter at least one reviewer's email address. Now
add any optional notes, and send your change off for review by clicking on
**Send**. Unless you send your change to reviewers, no one will know to look
at it.

_Note_: If you don't see editing commands on the review page, click **Sign in**
in the upper right. _Hint_: You can add -r reviewer@example.com --send-mail to
send the email directly when uploading a change using git-cl.


The review process
------------------

If you submit a giant patch, or do a bunch of work without discussing it with
the relevant people, you may have a hard time convincing anyone to review it!

Code reviews are an important part of the engineering process. The reviewer will
almost always have suggestions or style fixes for you, and it's important not to
take such suggestions personally or as a commentary on your abilities or ideas.
This is a process where we work together to make sure that the highest quality
code gets submitted!

You will likely get email back from the reviewer with comments. Fix these and
update the patch set in the issue by uploading again. The upload will explain
that it is updating the current CL and ask you for a message explaining the
change. Be sure to respond to all comments before you request review of an
update.

If you need to update code the code on an already uploaded CL, simply edit the
code, commit it again locally, and then run git cl upload again e.g.

    echo "GOATS" > whitespace.txt
    git add whitespace.txt
    git commit -m 'add GOATS fix to whitespace.txt'
    git cl upload

Once you're ready for another review, use **Reply** again to send another
notification (it is helpful to tell the review what you did with respect to each
of their comments). When the reviewer is happy with your patch, they will
approve your change by setting the Code-Review label to "+1".

_Note_: As you work through the review process, both you and your reviewers
should converse using the code review interface, and send notes.

Once your change has received an approval, you can click the "Submit to CQ"
button on the codereview page and it will be committed on your behalf.

Once your commit has gone in, you should delete the branch containing your change:

    git checkout -q origin/master
    git branch -D my_feature


Final Testing
-------------

Skia's principal downstream user is Chromium, and any change to Skia rendering
output can break Chromium. If your change alters rendering in any way, you are
expected to test for and alleviate this. (You may be able to find a Skia team
member to help you, but the onus remains on each individual contributor to avoid
breaking Chrome.

### Evaluating Impact on Chromium

Keep in mind that Skia is rolled daily into Blink and Chromium.  Run local tests
and watch canary bots for results to ensure no impact.  If you are submitting
changes that will impact layout tests, follow the guides below and/or work with
your friendly Skia-Blink engineer to evaluate, rebaseline, and land your
changes.

Resources:

[How to land Skia changes that change Blink layout test results](../chrome/layouttest)

If you're changing the Skia API, you may need to make an associated change in Chromium.  
If you do, please follow these instructions: [Landing Skia changes which require Chrome changes](../chrome/changes)


Check in your changes
---------------------

### Non-Skia-committers

If you already have committer rights, you can follow the directions below to
commit your change directly to Skia's repository.

If you don't have committer rights in https://skia.googlesource.com/skia.git ...
first of all, thanks for submitting your patch!  We really appreciate these
submissions.  After receiving an approval from a committer, you will be able to
click the "Submit to CQ" button and submit your patch via the commit queue.  

In special instances, a Skia committer may assist you in landing the change
by uploading a new codereview containing your patch (perhaps with some small
adjustments at his/her discretion).  If so, you can mark your change as
"Abandoned", and update it with a link to the new codereview.

### Skia committers 
  *  tips on how to apply an externally provided patch are [here](./patch)
  *  when landing externally contributed patches, please note the original
     contributor's identity (and provide a link to the original codereview) in the commit message

    git-cl will squash all your commits into a single one with the description you used when you uploaded your change.

    ~~~~
    git cl land
    ~~~~
    
    or
    
    ~~~~
    git cl land -c 'Contributor Name <email@example.com>'
    ~~~~
