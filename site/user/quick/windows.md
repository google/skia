Windows
=======

Prerequisites
-------------

Make sure the following have been installed:

  * Visual C++ 2013 Express or later, available for free
    * http://www.visualstudio.com/downloads/download-visual-studio-vs#d-express-windows-8

  * Chromium depot_tools
    * http://www.chromium.org/developers/how-tos/depottools
  * git
    * Either use the mysygit installed by depot_tools
    * Or install msys git: http://msysgit.github.io/ yourself
  * Python 2.7.x (if you're not planning to use cygwin)
    * available for free download at http://www.python.org/download/releases/
    * make sure the installer changes your %PATH% environment variable to include the directory with the "python.exe" binary
  * Cygwin (optional, but useful if you want to use git)
    * Download from  http://cygwin.org/setup.exe
    * use any mirror you like; http://lug.mtu.edu works well
    * Cygwin installs a minimum of options. Add these packages if they aren't already selected:
      * Devel git
      * Devel subversion
      * Editors vim
        * to fix arrows in insert, copy /usr/share/vim/vim73/vimrc_example.vim to ~/.vimrc
      * Net ca-certificates
      * Python python
      * Utils patch
      * Utils util-linux
    * set the windows envionment variable CYGWIN to nodosfilewarning

Check out the source code
-------------------------

see https://skia.org/user/download

Generate Visual Studio projects
-------------------------------

We use the open-source gyp tool to generate Visual Studio projects (and
analogous build scripts on other platforms) from our multi-platform "gyp"
files.

You can generate the Visual Studio projects by running gyp_skia, as follows:

    (setup GYP_GENERATORS, see just below)
    cd %SKIA_CHECKOUT_DIR%
    python gyp_skia

If you like to develop from a fully-integrated Visual Studio, set
GYP_GENERATORS=ninja,msvs-ninja before running gyp_skia to generate
Visual-Studio-compatible project files that still ultimately build using ninja,
or set it to msvs to use Visual Studio's own (slower) build system.  If you are
comfortable with and prefer running ninja yourself, GYP_GENERATORS=ninja is
considerably faster than the other two options.

Build and run tests from the command line
-----------------------------------------

    ninja -C out\Debug dm
    out\Debug\dm

Normally you should run tests in Debug mode (SK_DEBUG is defined, and debug
symbols are included in the binary). If you would like to build the Release
version instead:

    ninja -C out\Release dm
    out\Release\dm

Build and run tests in the Visual Studio IDE
--------------------------------------------

  * Generate the Visual Studio project files by running gyp_skia as described above
  * Open a File Explorer window pointing at the %SKIA_CHECKOUT_DIR%\out\gyp directory
  * Double-click on dm.sln to start Visual Studio and load the project
  * When Visual Studio starts, you may see an error dialog stating that "One or more projects in the solution were not loaded correctly"... but there's probably nothing to worry about.
  * In the "Solution Explorer" window, right-click on the "dm" project and select "Set as StartUp Project".
  * In the "Debug" menu, click on "Start Debugging" (or just press F5). If you get a dialog saying that the project is out of date, click on "Yes" to rebuild it.
  * Once the build is complete, you should see console output from the tests in the "Output" window at lower right.

Build and run SampleApp in Visual Studio
----------------------------------------

  * Generate the Visual Studio project files by running gyp_skia as described above
  * Open a File Explorer window pointing at the %SKIA_INSTALLDIR%\trunk\out\gyp directory
  * Double-click on SampleApp.sln
  * When Visual Studio starts, you may see an error dialog stating that "One or more projects in the solution were not loaded correctly"... but there's probably nothing to worry about.
  * In the "Debug" menu, click on "Start Debugging" (or just press F5). If you get a dialog saying that the project is out of date, click on "Yes" to rebuild it.
  * Once the build is complete, you should see a window with various example graphics. To move through the sample app, use the following keypresses:
    * right-arrow key: cycle through different test pages
    * left-arrow key: cycle through rendering methods for each test page
    * other keys are defined in SampleApp.cpp’s SampleWindow::onHandleKey() and SampleWindow::onHandleChar() methods

Build and run nanobench (performance testbench) from the command line
---------------------------------------------------------------------

Since nanobench tests performance, it usually makes more sense to run it in Release mode.

    ninja -C out\Release nanobench
    out\Release\nanobench
