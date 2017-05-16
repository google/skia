* Introduction:
  =============

JSON (JavaScript Object Notation) is a lightweight data-interchange format. 
It can represent integer, real number, string, an ordered sequence of 
value, and a collection of name/value pairs.

JsonCpp (http://jsoncpp.sourceforge.net/) is a simple API to manipulate 
JSON value, handle serialization and unserialization to string.

It can also preserve existing comment in unserialization/serialization steps,
making it a convenient format to store user input files.

Unserialization parsing is user friendly and provides precise error reports.

* Using json-cpp in your project:
  ===============================

The recommended approach to integrate json-cpp in your project is to
build the the amalgamated source (a single .cpp) with your own build
system. This ensures compilation flags consistency and ABI compatibility.

See section "Generating amalgamated source and header" to generate them
from the source distribution.
  
Directory include/ should be added to your compiler include path. 
json-cpp headers should be included as follow:

  #include <json/json.h>
  
If json-cpp was build as a dynamic library on Windows, then your project
need to define macro "JSON_DLL" to JSON_API should import exported symbols.

* Building/Testing with new CMake build system:
  =============================================
  
CMake is a C++ Makefiles/Solution generator that can be downloaded from:
  http://www.cmake.org
  
It is usually available on most Linux system as package. On Ubuntu:
  sudo apt-get install cmake
  
Notes that python is also required to run JSON reader/writer tests. If
missing, the build will skip running those tests.
  
When running CMake, a few parameters are required:
- a build directory where the makefiles/solution are generated. It is
  also used to store objects, libraries and executables files.
- the generator to use: makefiles or Visual Studio solution? What version
  or Visual Studio, 32 or 64 bits solution? 
  
Generating solution/makefiles using cmake-gui:
- Makes "source code" points the source directory
- Makes "where to build the binary" points to the directory to use for 
  the build.
- Click on the "Grouped" check box
- Review JsonCpp build option (tick JSONCPP_LIB_BUILD_SHARED to build as 
  a dynamic library)
- Click configure button at the bottom, then the generate button.
- The generated solution/makefiles can be found in the binary directory.
  
Alternatively, from the command-line on Unix in the source directory:
  
  mkdir -p ../build/debug
  cd ../build/debug
  cmake -DCMAKE_BUILD_TYPE=debug -DJSONCPP_LIB_BUILD_SHARED=OFF -G "Unix Makefiles" ../../jsoncpp-src
  make
  
Running "cmake -h" will display the list of available generators (passed as -G option).
  
By default CMake hides compilation command-line. This can be modified by specifying:
-DCMAKE_VERBOSE_MAKEFILE=true when generating makefiles.

* Building/Testing with the legacy build system based on SCons:
  =============================================================

JsonCpp uses Scons (http://www.scons.org) as a build system. Scons requires
python to be installed (http://www.python.org).

You download scons-local distribution from the following url:
http://sourceforge.net/projects/scons/files/scons-local/1.2.0/

Unzip it in the directory where you found this README file. scons.py Should be 
at the same level as README.

python scons.py platform=PLTFRM [TARGET]
where PLTFRM may be one of:
	suncc Sun C++ (Solaris)
	vacpp Visual Age C++ (AIX)
	mingw 
	msvc6 Microsoft Visual Studio 6 service pack 5-6
	msvc70 Microsoft Visual Studio 2002
	msvc71 Microsoft Visual Studio 2003
	msvc80 Microsoft Visual Studio 2005
	msvc90 Microsoft Visual Studio 2008
	linux-gcc Gnu C++ (linux, also reported to work for Mac OS X)

Notes: if you are building with Microsoft Visual Studio 2008, you need to 
setup the environment by running vcvars32.bat (e.g. MSVC 2008 command prompt)
before running scons.
	
Adding platform is fairly simple. You need to change the Sconstruct file 
to do so.
	
and TARGET may be:
	check: build library and run unit tests.
    
* Running the test manually:
  ==========================

Notes that test can be run by scons using the 'check' target (see above).

You need to run test manually only if you are troubleshooting an issue.

In the instruction below, replace "path to jsontest.exe" with the path
of the 'jsontest' executable that was compiled on your platform.
  
cd test
# This will run the Reader/Writer tests
python runjsontests.py "path to jsontest.exe"

# This will run the Reader/Writer tests, using JSONChecker test suite
# (http://www.json.org/JSON_checker/).
# Notes: not all tests pass: JsonCpp is too lenient (for example,
# it allows an integer to start with '0'). The goal is to improve
# strict mode parsing to get all tests to pass.
python runjsontests.py --with-json-checker "path to jsontest.exe"

# This will run the unit tests (mostly Value)
python rununittests.py "path to test_lib_json.exe"

You can run the tests using valgrind:
python rununittests.py --valgrind "path to test_lib_json.exe"


* Building the documentation:
  ===========================

Run the python script doxybuild.py from the top directory:

python doxybuild.py --open --with-dot

See doxybuild.py --help for options. 

Notes that the documentation is also available for download as a tarball. 
The documentation of the latest release is available online at:
http://jsoncpp.sourceforge.net/

* Generating amalgamated source and header
  ========================================

JsonCpp is provided with a script to generate a single header and a single
source file to ease inclusion in an existing project.

The amalgamated source can be generated at any time by running the following
command from the top-directory (requires python 2.6):

python amalgamate.py

It is possible to specify header name. See -h options for detail. By default,
the following files are generated:
- dist/jsoncpp.cpp: source file that need to be added to your project
- dist/json/json.h: header file corresponding to use in your project. It is
equivalent to including json/json.h in non-amalgamated source. This header
only depends on standard headers. 
- dist/json/json-forwards.h: header the provides forward declaration
of all JsonCpp types. This typically what should be included in headers to
speed-up compilation.

The amalgamated sources are generated by concatenating JsonCpp source in the
correct order and defining macro JSON_IS_AMALGAMATION to prevent inclusion
of other headers.

* Adding a reader/writer test:
  ============================

To add a test, you need to create two files in test/data:
- a TESTNAME.json file, that contains the input document in JSON format.
- a TESTNAME.expected file, that contains a flatened representation of 
  the input document.
  
TESTNAME.expected file format:
- each line represents a JSON element of the element tree represented 
  by the input document.
- each line has two parts: the path to access the element separated from
  the element value by '='. Array and object values are always empty 
  (e.g. represented by either [] or {}).
- element path: '.' represented the root element, and is used to separate 
  object members. [N] is used to specify the value of an array element
  at index N.
See test_complex_01.json and test_complex_01.expected to better understand
element path.


* Understanding reader/writer test output:
  ========================================

When a test is run, output files are generated aside the input test files. 
Below is a short description of the content of each file:

- test_complex_01.json: input JSON document
- test_complex_01.expected: flattened JSON element tree used to check if 
    parsing was corrected.

- test_complex_01.actual: flattened JSON element tree produced by 
    jsontest.exe from reading test_complex_01.json
- test_complex_01.rewrite: JSON document written by jsontest.exe using the
    Json::Value parsed from test_complex_01.json and serialized using
    Json::StyledWritter.
- test_complex_01.actual-rewrite: flattened JSON element tree produced by 
    jsontest.exe from reading test_complex_01.rewrite.
test_complex_01.process-output: jsontest.exe output, typically useful to
    understand parsing error.

* License
  =======
  
See file LICENSE for details. Basically JsonCpp is licensed under 
MIT license, or public domain if desired and recognized in your jurisdiction.

