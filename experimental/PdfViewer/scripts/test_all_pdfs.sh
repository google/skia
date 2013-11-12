#!/bin/bash

# Copyright 2013 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if [ "${1}" == '-h' ] || [ "${1}" == '--help' ]
then
echo 'test_all_pdfs.sh
Usage: Run from the trunk directory of a Skia tree to convert a folder of PDFs into PNGs.
       Many threads will be run simultaneously using the "parallel" tool if that tool is
       installed. "parallel" can be installed on linux using "sudo apt-get install parallel."
       Requires that skdiff and pdfviewer have already been built.

Arguments:
    folder      Folder containing PDF files. The PNG results will be placed in "folder/new".
                If folder contains a folder named "old", the PNGs in "folder/new" will be
                compared to PNGs in "folder/old" with the same name, and the differences
                will be stored in a "folder/d".'
exit
fi

# Early exit if pdfviewer has not been built.
# TODO (scroggo): Use release version if debug is unavailable
if [ ! -f out/Debug/pdfviewer ]
then
    echo 'debug version of pdfviewer is required.'
    exit
fi

if [ -z $1 ]
then
    echo 'folder is a required argument.'
    exit
fi

if [ ! -d $1 ]
then
    echo 'folder must be a valid directory.'
    exit
fi

# Create the directory to contain the new results. If old new results exist, remove them.
if [ -d $1/new ]
then
    rm -rf $1/new
fi

mkdir $1/new/

# Run the script to read each PDF and convert it to a PNG.
if command -v parallel >/dev/null 2>&1
then
    echo 'Running in parallel'
    ls -1 $1/*.pdf | sed "s/^/experimental\/PdfViewer\/scripts\/vm_pdf_viewer_run_one_pdf.sh /" \
        | parallel
else
    echo 'Converting each file sequentially. Install "parallel" to convert in parallel.'
    echo '"parallel" can be installed on linux with "sudo apt-get install parallel".'
    ls -1 $1/*.pdf | xargs experimental/PdfViewer/scripts/vm_pdf_viewer_run_one_pdf.sh
fi

# Next, compare to the old results. Exit now if there is no folder with old results.
if [ ! -d $1/old ]
then
    exit
fi

# Check to make sure that skdiff has been built.
RELEASE_SKDIFF=out/Release/skdiff
DEBUG_SKDIFF=out/Debug/skdiff
if [ -f $RELEASE_SKDIFF ]
then
    SKDIFF=$RELEASE_SKDIFF
elif [ -f $DEBUG_SKDIFF ]
then
    SKDIFF=$DEBUG_SKDIFF
else
    echo 'Build skdiff in order to do comparisons.'
    exit
fi

# Create the diff folder, after deleting old diffs if necessary.
if [ -d $1/d ]
then
    rm -rf $1/d
fi

mkdir $1/d

$SKDIFF $1/old $1/new $1/d
