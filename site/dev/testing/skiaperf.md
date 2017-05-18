Skia Perf
=========

[Skia Perf](https://perf.skia.org) is a Polymer-based web application for
analyzing and viewing performance metrics produced by Skia's testing
infrastructure.

<img src=Perf.png style="margin-left:30px" align="left" width="800"/> <br clear="left">

Skia tests across a large number of platforms and configurations, and each
commit to Skia generates 240,000 individual values are sent to Perf,
consisting mostly of performance benchmark results, but also including memory
and coverage data.

Perf includes tools for analyzing such a large corpus of data, the most
powerful is [k-means clustering](https://perf.skia.org/t/). This tool groups
large sets of performance metrics together based on how they change over time,
and highlights sets of metrics that have performance regressions.

<img src=Cluster.png style="margin-left:30px" align="left" width="500"/> <br clear="left">

Calculations
------------

Skia Perf has the ability to perform calculations over the test data
allowing you to build up interesting queries.

This query displays [the ratio of playback time in ms to the number of ops for desk\_wowwiki.skp](https://perf.skia.org/#1876):

    ratio(
      ave(fill(filter("name=desk_wowwiki.skp&sub_result=min_ms"))),
      ave(fill(filter("name=desk_wowwiki.skp&sub_result=ops")))
    )

You can also use the data to answer questions like [how many tests were run per commit](https://perf.skia.org/#1878).

    count(filter(""))

See Skia Perf for the [full list of functions available](https://perf.skia.org/help).

