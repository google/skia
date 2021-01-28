SVG Tools
=========

This directory contains the following-


svgs.txt
--------
This text file contains an SVG URL per line.
It is a list of the SVG files used to test rendering correctness.

svg_images.txt
--------------
This text file contains an image URL per line.
It is a list of images used by the SVGs in svgs.txt.

svgs_parse_only.txt
-------------------
This text file contains an SVG URL per line.
It is a list of the SVG files used to exercise the SVG parsing code.

svg_downloader.py
-----------------
This python script parses txt files and downloads SVGs and images into a specified directory.

The script can be run by hand:
$ python svg_downloader.py --output_dir /tmp/svgs/
OR
$ python svg_downloader.py --output_dir /tmp/svgs/ --input_file svgs_parse_only.txt --prefix svgparse_

If the --keep_common_prefix argument is specified, URL components after the common prefix
will be preserved in the destination directory hierarchy. For example, if the input file contains
URLs https://example.com/images/a.png and https://example.com/images/subdir/b.png, the downloaded
files will go to output_dir/a.png and output_dir/subdir/b.png.