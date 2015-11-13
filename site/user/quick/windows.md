Windows
=======

Prerequisites
-------------

Make sure the following have been installed:

*   [Visual C++ 2013 Express or later, available for
    free](https://www.visualstudio.com/downloads/download-visual-studio-vs#d-express-windows-8)

*   [Chromium depot_tools](https://www.chromium.org/developers/how-tos/depottools)

*   Git
    -   Either use the mysygit installed by depot_tools
    -   Or [install git-for-windows
        yourself](https://git-for-windows.github.io/).

*   Python 2.7.x (if you're not planning to use cygwin)
    -   available for free download at
        [python.org](https://www.python.org/download/releases/)
    -   make sure the installer changes your %PATH% environment variable
        to include the directory with the "python.exe" binary

*   Cygwin (**optional**, but useful if you want to use git)
    -   Download from https://www.cygwin.com/setup-x86.exe or
        https://www.cygwin.com/setup-x86_64.exe
    -   Use any mirror you like; http://lug.mtu.edu works well
    -   Cygwin installs a minimum of options. Add these packages if they
        aren't already selected:
        +   Devel git
        +   Devel subversion
        +   Editors vim
            *   to fix arrows in insert, copy
                `/usr/share/vim/vim73/vimrc_example.vim` to `~/.vimrc`
        +   Net ca-certificates
        +   Python python
        +   Utils patch
        +   Utils util-linux
    -   Set the windows envionment variable CYGWIN to nodosfilewarning

Check out the source code
-------------------------

Follow the instructions [here](../download) for downloading the Skia source.

Gyp Generators on Windows
-------------------------

We use the open-source Gyp tool to generate Visual Studio projects (and
analogous build scripts on other platforms) from our multi-platform "gyp"
files.

Three Gyp generators are used on Windows:

*   `ninja` - Run ninja yourself, without VisualStudio  project files,

*   `msvs-ninja` - Develop from a fully-integrated Visual Studio.
    Gyp generates Visual-Studio-compatible project files that still
    ultimately build using ninja

*   `msvs` - Use Visual Studio's own (slower) build system

To choose which ones to use, set the `GYP_GENERATORS` environment
variable to a comma-delimited list of generators before running
sync-and-gyp. The default value for `GYP_GENERATORS` is
`ninja,msvs-ninja`.  For example to enable the `ninja` and `msvs`
generators:

<a name="env"></a>Setting Enviroment Variables in Windows CMD.EXE
-----------------------------------------------------------------

    cd %SKIA_CHECKOUT_DIR%
    SET "GYP_GENERATORS=ninja,msvs"
    python bin/sync-and-gyp
    SET "GYP_GENERATORS="

Build and run tests from the command line
-----------------------------------------

    ninja -C out/Debug dm
    out\Debug\dm

See [this page for running Skia tests on all desktop](./desktop)

Build and run tests in the Visual Studio IDE
--------------------------------------------

1.  Generate the Visual Studio project files by running `sync-and-gyp` as
    described above

2.  Open a File Explorer window pointing at the
    `%SKIA_CHECKOUT_DIR%\out\gyp` directory

3.  Double-click on dm.sln to start Visual Studio and load the project

4.  When Visual Studio starts, you may see an error dialog stating that
    "One or more projects in the solution were not loaded
    correctly"... but there's probably nothing to worry about.

5.  In the "Solution Explorer" window, right-click on the "dm" project
    and select "Set as StartUp Project".

6.  In the "Debug" menu, click on "Start Debugging" (or just press
    F5). If you get a dialog saying that the project is out of date,
    click on "Yes" to rebuild it.

7.  Once the build is complete, you should see console output from the
    tests in the "Output" window at lower right.

Build and run SampleApp in Visual Studio
----------------------------------------

1.  Generate the Visual Studio project files by running `sync-and-gyp`
    as described above

2.  Open a File Explorer window pointing at the
    `%SKIA_INSTALLDIR%\trunk\out\gyp` directory

3.  Double-click on SampleApp.sln

4.  When Visual Studio starts, you may see an error dialog stating
    that "One or more project s in the solution were not loaded
    correctly"... but there's probably nothing to worry about.

5.  In the "Debug" menu, click on "Start Debugging" (or just press
    F5). If you get a dialog saying that the project is out of date,
    click on "Yes" to rebuild it.

6.  Once the build is complete, you should see a window with various
    example graphics. To move through the sample app, use the
    following keypresses:
    -    right-arrow key: cycle through different test pages
    -    left-arrow key: cycle through rendering methods for each test page
    -    other keys are defined in SampleApp.cpp’s
         SampleWindow::onHandleKey() and SampleWindow::onHandleChar()
         methods
