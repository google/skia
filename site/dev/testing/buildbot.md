Skia Buildbots
==============

Overview
--------

Like the Chromium team, the Skia team uses [buildbot](http://trac.buildbot.net/)
to run continuous builds and tests.

Here is a link to our main status page: https://status.skia.org/

There are also Skia client, compile, Android, and FYI console pages for a detailed
view of those results: 
  
  Externally-facing: http://build.chromium.org/p/client.skia/console
  
  Internally-facing: http://chromegw.corp.google.com/i/client.skia/console
                     http://chromegw.corp.google.com/i/client.skia.internal/console
                     \(only visible internally\)

Architecture
------------

The buildbot system consists of these elements: \(see
http://buildbot.net/buildbot/docs/current/manual/introduction.html#system-architecture
for more detail\) 

* builder

    * one repeatable build and/or test configuration on a given platform.
    * each builder maintains its own local checkout of the Skia repo
    * only one builder is running at any given time on any single buildslave; otherwise,
       different builders could interfere with each other's performance numbers

* buildbot master
    
    * watches for new commits to land in the Skia repository 
      \(https://skia.googlesource.com/skia\) 
    * whenever a new commit lands, it tells buildbot slaves to start building and 
      testing the latest revision 
    * serves up status pages whenever anybody requests them

* buildslave \(or "buildbot slave"\)
    
    * a process on a machine that builds and runs code as directed by the buildbot 
      master
    * one or more builders run on each buildslave

* build

    * one run of a particular builder, at a particular code revision


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




