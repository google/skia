---
title: 'Contributing to Skia'
linkTitle: 'Contributing'

weight: 1
menu:
  main:
    weight: 40
---

Here some ways you can get involved and help us improve Skia.

## Report Bugs

Find bugs to fix or report new bugs in the
[Skia issue tracker](http://bug.skia.org/). You can also search the
[Chromium issue tracker](http://code.google.com/p/chromium/issues/list) for bugs
related to graphics or Skia.

## Test

Write an application or tool that will exercise the Skia code differently than
our current set of tests and verify that Skia works as expected. Draw something
interesting and profile it to find ways to speed up Skia's implementation.We
cannot always fix issues or support every scenario, but we welcome any bugs
found so we can assess and prioritize them. (If you find _and_ fix a bug, even
better!)

## Contribute Code

Whether you develop a new feature or a fix for an existing bug in the Skia code
base, you will need a committer to review and approve the change. There are some
steps that can speed up the review process:

- Keep your code submissions small and targeted.
- When possible, have a fellow contributor review your change in advance of
  submission.
- Propose new features to the project leads by opening a feature bug or posting
  to skia-discuss ahead of development.

For more information, see [How to submit a patch](/docs/dev/contrib/submit/).

For background on the project and an outline of the types of roles interested
parties can take on, see [Project Roles](/docs/roles).

Anyone contributing code to Skia must sign a Contributor License Agreement and
ensure they are listed in the AUTHORS file:

- Individual contributors can complete the
  [Individual Contributor License Agreement](https://developers.google.com/open-source/cla/individual)
  online.
- If you are contributing on behalf of a corporation, fill out the
  [Corporate Contributor License Agreement](https://developers.google.com/open-source/cla/corporate)
  and send it in as described on that page.

- If it is your first time submitting code or you have not previously done so,
  add your (or your organization's) name and contact info to the
  [AUTHORS file](https://skia.googlesource.com/skia/+/main/AUTHORS) as a part
  of your CL.

REVIEWERS: Before you LGTM a change, verify that the contributor is listed in
the AUTHORS file.

If they are not, a Googler must ensure that the individual or their corporation
has signed the CLA by searching
[go/cla-signers](https://goto.google.com/cla-signers). Then have an entry added
to the AUTHORS file with the CL.
