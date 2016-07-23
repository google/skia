Skia Buildbot Scripts
=====================

The scripts in this directory are ported from Skia's buildbot recipes and are
intended to run as standalone Python scripts either locally or via Swarming.

How to Run
----------

The scripts can be run by hand, eg:

$ cd infra/bots
$ python compile_skia.py Build-Ubuntu-GCC-x86_64-Debug ../../out

Or, you can run the scripts via Swarming:

$ isolate archive --isolate-server https://isolateserver.appspot.com/ -i infra/bots/compile_skia.isolate -s ../compile-skia.isolated --verbose --config-variable BUILDER_NAME=Build-Ubuntu-GCC-x86_64-Debug
$ swarming.py run --swarming https://chromium-swarm.appspot.com --isolate-server https://isolateserver.appspot.com --dimension os Ubuntu --dimension pool Skia --task-name compile-skia --io-timeout=3600 --hard-timeout=3600 ../compile-skia.isolated
