SVG Tools
=========

This directory contains the following-


svgs.txt
--------
This txt file contains an SVG URL per line.
The SVGs in this file have been downloaded from the internal doc here:
https://docs.google.com/document/d/1ej-xgkzW-9kXkpA0rbVQ5zOBRRDz_QrQEkGVbefW8Ys/edit

svgs_parse_only.txt
-------------------
This text file contains an SVG URL per line.
The SVGs in this file have been downloaded from the internal doc here (excluding the SVGs in svgs.txt):
https://docs.google.com/document/d/1kYRvUxZTnm1tI_0bTU0BX9jqSSTqPUhGXJVcD3Rcg2c/edit


svg_downloader.py
-----------------
This python script parses txt files and downloads SVGs into a specified directory.

The script can be run by hand:
$ python svg_downloader.py --output_dir /tmp/svgs/
OR
$ python svg_downloader.py --output_dir /tmp/svgs/ --svgs_file svgs_parse_only.txt --prefix svgparse_

