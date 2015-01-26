Skia Perf
=========

[Skia Perf](https://perf.skia.org) is a web based interface for exploring
performance data produced by `nanobench` and the code size bot. The data
includes:

  * The nanobench test times in ms.
  * Total memory consumed during a nanobench run.
  * Code size for various symbol types in bytes.

All of the data can be plotted and also can be [analyzed using k-means
clustering](https://perf.skia.org/clusters/).

Calculations
------------

Skia Perf has the ability to perform calculations over the test data
allowing you to build up interesting queries.

For example, this query displays the [total code size of the library over time](https://perf.skia.org/#1877):

    sum(fill(filter("config=memory&sub_result=bytes")))

This query displays [the ratio of playback time in ms to the number of ops for desk\_wowwiki.skp](https://perf.skia.org/#1876):

    ratio(
      ave(fill(filter("name=desk_wowwiki.skp&sub_result=min_ms"))),
      ave(fill(filter("name=desk_wowwiki.skp&sub_result=ops")))
    )

You can also use the data to answer questions like [how many tests were run per commit](https://perf.skia.org/#1878).

    count(filter(""))

See Skia Perf for the [full list of functions available](https://perf.skia.org/help).
