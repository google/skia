Skia Buildbots
==============

Overview
--------

Like the Chromium team, the Skia team uses [buildbot](http://trac.buildbot.net/)
to run continuous builds and tests.

Here is a link to our main status page: https://status.skia.org/

There are also buildbot console pages for a detailed view of those results:
  
  Externally-facing:

* http://build.chromium.org/p/client.skia/console
* http://build.chromium.org/p/client.skia.android/console
* http://build.chromium.org/p/client.skia.compile/console
* http://build.chromium.org/p/client.skia.fyi/console

  Internally-facing:

* http://uberchromegw.corp.google.com/i/client.skia/console
* http://uberchromegw.corp.google.com/i/client.skia.android/console
* http://uberchromegw.corp.google.com/i/client.skia.compile/console
* http://uberchromegw.corp.google.com/i/client.skia.fyi/console
* http://uberchromegw.corp.google.com/i/client.skia.internal/console


Architecture
------------

The buildbot system consists of these elements: \(see
http://buildbot.net/buildbot/docs/current/manual/introduction.html#system-architecture
for more detail\) 

* Buildbot Master

    * Watches for new commits to land in the Skia repository
      \(https://skia.googlesource.com/skia\)
    * Whenever a new commit lands, it triggers a **Build** on each **Builder**
      to test the new revision.
    * Serves up status pages whenever anybody requests them

* Build

    * One run of a particular **Builder**, at a particular code revision.
    * "Build" is sort of a misnomer; it's just a list of steps (typically shell
      commands) which are run by the **Buildslave** process on the host
      machine, and those may include compiling and running code as well as
      arbitrary other commands.

* Builder

    * One repeatable build and/or test configuration on a given platform. The
      Builder is basically a blueprint which provides logic to determine which
      steps to run within a Build.

* Buildslave \(or "buildbot slave"\)
    
    * A process running on a host machine that builds and runs code as directed
      by the Buildbot Master.
    * One or more Builders may run on a given Buildslave, but only one runs at
      a time.
    * One or more Buildslaves may run on a given host machine.


Status View
------------

The status view shows a table with builders, grouped by test type and platform,
on the X-axis and commits on the Y-axis.  The cells are colored according to
the status of the build for each commit: green indicates success, red indicates
failure, light orange indicates an in-progress build, and white indicates that
no build has started yet for a given revision. Commits are listed by author, and
the branch on which the commit was made is shown on the very left.

For more detail, you can click on an individual cell to get a summary of the
steps which ran for that build.  You can also click one of the white bars at
the top of each column to see a summary of recent builds for a given builder.




