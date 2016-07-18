Blacklists for CT bots
======================

Files in this directory are used by the [ct_skps.py](https://cs.chromium.org/chromium/build/scripts/slave/recipes/skia/ct_skps.py) recipe to find which SKPs should be blacklisted.

The format of the files are:

{
    "blacklisted_skps": [
        "example1.skp",
        "example2.skp"
    ]
}
