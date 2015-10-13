CMAKE_INSTALL_PREFIX
--------------------

Install directory used by install.

If "make install" is invoked or INSTALL is built, this directory is
prepended onto all install directories.  This variable defaults to
/usr/local on UNIX and c:/Program Files on Windows.

On UNIX one can use the DESTDIR mechanism in order to relocate the
whole installation.  DESTDIR means DESTination DIRectory.  It is
commonly used by makefile users in order to install software at
non-default location.  It is usually invoked like this:

::

 make DESTDIR=/home/john install

which will install the concerned software using the installation
prefix, e.g.  "/usr/local" prepended with the DESTDIR value which
finally gives "/home/john/usr/local".

WARNING: DESTDIR may not be used on Windows because installation
prefix usually contains a drive letter like in "C:/Program Files"
which cannot be prepended with some other prefix.

The installation prefix is also added to CMAKE_SYSTEM_PREFIX_PATH so
that find_package, find_program, find_library, find_path, and
find_file will search the prefix for other software.

.. note::

  Use the :module:`GNUInstallDirs` module to provide GNU-style
  options for the layout of directories within the installation.
