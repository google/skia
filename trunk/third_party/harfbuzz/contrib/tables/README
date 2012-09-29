This directory contains Python script to parse several of the Unicode tables
that are downloadable from the web and generate C header files from them.

These are the locations of the files which are parsed. You should download these
files and put them in this directory.

http://www.unicode.org/Public/5.1.0/ucd/extracted/DerivedGeneralCategory.txt
http://www.unicode.org/Public/5.1.0/ucd/extracted/DerivedCombiningClass.txt
http://www.unicode.org/Public/UNIDATA/auxiliary/GraphemeBreakProperty.txt
http://www.unicode.org/Public/5.1.0/ucd/Scripts.txt

Then you can run the following python scripts to generate the header files:

python category-parse.py DerivedGeneralCategory.txt category-properties.h
python combining-class-parse.py DerivedCombiningClass.txt combining-properties.h
python grapheme-break-parse.py GraphemeBreakProperty.txt grapheme-break-properties.h
python scripts-parse.py Scripts.txt script-properties.h
