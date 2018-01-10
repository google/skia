Applying patches
================

If you are a Skia committer and have been asked to commit an
externally-submitted patch, this is how to do it.  (This technique is useful in
other situations too, like if you just want to try out somebody else's patch
locally.)

Notes: 

  * For the examples below, we will assume that this is the change you want
    to patch into your local checkout: https://codereview.appspot.com/6201055/ 
  * These instructions should work on Mac or Linux; Windows is trickier, 
    because there is no standard Windows "patch" tool.  

See also [Contributing Code for The Chromium Projects]
(http://dev.chromium.org/developers/contributing-code#TOC-Instructions-for-Reviewer:-Checking-in-the-patch-for-a-non-committer).

If you use `git cl`, then you should be able to use the shortcut:

~~~~ 
git cl patch 6201055 
~~~~

If you use `gcl`, or the above doesn't work, the following should always work.

1. Prepare your local workspace to accept the patch.

    * cd into the root directory (usually `trunk/`) of the workspace where you
      want to apply the patch.  
    * Make sure that the workspace is up-to-date and clean (or "updated and 
      clean enough" for your purposes).  If the codereview patch was against 
      an old revision of the repo, you may need to sync your local workspace 
      to that same revision.

2. Download the raw patch set.

    * Open the codereview web page and look for the "Download raw patch set"
      link near the upper right-hand corner.  Right-click on that link and copy
      it to the clipboard.  (In my case, the link is
      https://codereview.appspot.com/download/issue6201055_1.diff ) 
    * If you are on Linux or Mac and have "curl" or "wget" installed, you can 
      download the patch from the command line:

    ~~~~ 
    curl https://codereview.appspot.com/download/issue6201055_1.diff
    --output patch.txt
    # or...
    wget https://codereview.appspot.com/download/issue6201055_1.diff
    --output-document=patch.txt 
    ~~~~

    * Otherwise, figure out some other way to download this file and save it as
      `patch.txt`

3. Apply this patch to your local checkout.

    * You should still be in the root directory of the workspace where you want
      to apply the patch.

    ~~~~ 
    patch -p1 <patch.txt 
    ~~~~

    * Then you can run `diff` and visually check the local changes.

4. Complications: If the patch fails to apply, the following may be happening:

    * Wrong revision.  Maybe your local workspace is not up to date?  Or maybe the
      patch was made against an old revision of the repository, and cannot be applied
      to the latest revision?  (In that case, revert any changes and sync your
      workspace to an older revision, then re-apply the patch.)

