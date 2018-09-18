SVG Tools
=========

This directory contains the following-


svgs.txt
--------
This text file contains an SVG URL per line.
It is a list of the SVG files used to test rendering correctness.

svgs_parse_only.txt
-------------------
This text file contains an SVG URL per line.
It is a list of the SVG files used to exercise the SVG parsing code.

svg_downloader.py
-----------------
This python script parses txt files and downloads SVGs into a specified directory.

The script can be run by hand:
$ python svg_downloader.py --output_dir /tmp/svgs/
OR
$ python svg_downloader.py --output_dir /tmp/svgs/ --svgs_file svgs_parse_only.txt --prefix svgparse_

