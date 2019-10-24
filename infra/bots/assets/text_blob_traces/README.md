TEXT BLOB TRACES
================

Upload
------

To upload a new version of the assets, first place the new version in the
directory `text_blob_traces`, then execute:

    infra/bots/assets/assets.py upload   -t text_blob_traces text_blob_traces

Then commit the file `infra/bots/assets/text_blob_traces/VERSION`

Download
--------

Execute:

    infra/bots/assets/assets.py download -t text_blob_traces text_blob_traces

Run Bench and Simulator
-----------------------

    tools/git-sync-deps
    bin/gn gen out/release --args='is_debug=false'
    ninja -C out/release nanobench blob_cache_sim

    out/release/nanobench -m SkDiffBench --texttraces text_blob_traces -q

    out/release/blob_cache_sim text_blob_traces/*
