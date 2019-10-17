Skia Perf
=========

[Skia Perf](https://perf.skia.org) is a web application for analyzing and
viewing performance metrics produced by Skia's testing infrastructure.

<img src=Perf.png style="margin-left:30px" align="left" width="800"/> <br clear="left">

Skia tests across a large number of platforms and configurations, and each
commit to Skia generates more than 400,000 individual values that are sent to
Perf, consisting mostly of performance benchmark results, but also including
memory and coverage data.

Perf offers clustering, which is a tool to pick out trends and patterns in large sets of traces.

<img src=Cluster.png style="margin-left:30px" align="left" width="400"/> <br clear="left">

And can generate alerts when those trends spot a regression:

<img src=Regression.png style="margin-left:30px" align="left" width="800"/> <br clear="left">


## Calculations

Skia Perf has the ability to perform calculations over the test data
allowing you to build up interesting queries.

This query displays the ratio of playback time in ms to the number of ops for desk\_wowwiki.skp:

    ratio(
      ave(fill(filter("name=desk_wowwiki.skp&sub_result=min_ms"))),
      ave(fill(filter("name=desk_wowwiki.skp&sub_result=ops")))
    )

You can also use the data to answer questions like how many tests were run per commit.

    count(filter(""))

See Skia Perf for the [full list of functions available](https://perf.skia.org/help/).
