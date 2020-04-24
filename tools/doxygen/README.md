Doxygen
=======

To generate all the documentation run the following
from this directory:

    doxygen Doxyfile

The resulting output goes to

    /tmp/doxygen

To view those file locally in your browser run:

    cd /tmp/doxygen/html; python -m SimpleHTTPServer 8000

and visit

    http://localhost:8000

If you want to have the documentation regenerated on every save then
you can install `entr` and run the following from this directory:

    find  ../../include/ ../../src/ . | entr doxygen ./Doxyfile

Install
-------

For a linux desktop you can install the doxygen tool via:

    sudo apt install doxygen
