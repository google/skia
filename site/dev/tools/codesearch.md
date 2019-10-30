Code Search
===========

There are a number of ways to search the Skia codebase, each with advantages and
disadvantages.

[cs.skia.org](http://cs.skia.org) redirects to
[Chromium code search](https://code.google.com/p/chromium/codesearch) restricted
to the Skia portion of the Chromium tree. You can add a query after the slash;
e.g. [cs.skia.org/foo](http://cs.skia.org/foo) will search for "foo" within the
Skia tree. Chromium code search provides cross-references.

For Googlers, there is also the option of [the skia depot](http://cs/#skia/) in
internal Code Search. In addition to the
main [skia](http://cs/#skia/skia/) repo, internal Code Search indexes the
[buildbot](http://cs/#skia/buildbot/), [common](http://cs/#skia/common/),
and [skia_internal](https://cs/#skia/skia_internal/) repos. However,
cross-references and code analysis are not available.

The Github mirrors of the [skia](https://github.com/google/skia) and
[skia-buildbot](https://github.com/google/skia-buildbot) repos are useful for
investigating history and blame, or for exploring release branches or other
branches. However, the search functionality is fairly limited, cross-references
are not available, and in history the original committer's username is replaced
with that person's Github username.

You can also navigate through the
[Skia repos on googlesource.com](https://skia.googlesource.com/). All commits
appear here first.

  Code search option  |Search |XRef |History |Repos                         |Branches |Freshness
  --------------------|-------|-----|--------|------------------------------|---------|---------------
  [cs.skia.org][1]    |regexp | yes |yes     |skia                          |master   |last DEPS roll
  [Internal][2]       |regexp | no  |yes     |skia buildbot common internal |master   |hours
  [Github][3]         |basic  | no  |yes     |skia buildbot                 |all      |hour
  [googlesource][4]   |none   | no  |yes     |all                           |all      |N/A

[1]: http://cs.skia.org/             "Chromium code search"
[2]: http://cs/#skia/                "Internal Code Search"
[3]: https://github.com/google/skia  "Github mirror of skia"
[4]: https://skia.googlesource.com/  "Master Skia repos on googlesource.com"

Client code search
------------------

There is an [internal tool for
Googlers](https://goto.google.com/skia-client-search) to make it easier to
search the repos of Skia clients, e.g. Chromium, Android, and Mozilla. If you
use it and have suggestions, please let dogben know.
