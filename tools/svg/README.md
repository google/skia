SVG Tools
=========

This directory contains the following-


svgs.txt
--------
This text file contains an SVG URL per line.
The SVGs in this file have been downloaded from the internal doc here:
https://docs.google.com/document/d/1kYRvUxZTnm1tI_0bTU0BX9jqSSTqPUhGXJVcD3Rcg2c/edit


svg_downloader.py
-----------------
This python script parses svgs.txt and downloads SVGs into a specified directory.

The script can be run by hand:
$ python svg_downloader.py --output_dir /tmp/svgs/
