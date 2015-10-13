#.rst:
# CPackPackageMaker
# -----------------
#
# PackageMaker CPack generator (Mac OS X).
#
# Variables specific to CPack PackageMaker generator
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
# The following variable is specific to installers built on Mac
# OS X using PackageMaker:
#
# .. variable:: CPACK_OSX_PACKAGE_VERSION
#
#  The version of Mac OS X that the resulting PackageMaker archive should be
#  compatible with. Different versions of Mac OS X support different
#  features. For example, CPack can only build component-based installers for
#  Mac OS X 10.4 or newer, and can only build installers that download
#  component son-the-fly for Mac OS X 10.5 or newer. If left blank, this value
#  will be set to the minimum version of Mac OS X that supports the requested
#  features. Set this variable to some value (e.g., 10.4) only if you want to
#  guarantee that your installer will work on that version of Mac OS X, and
#  don't mind missing extra features available in the installer shipping with
#  later versions of Mac OS X.

#=============================================================================
# Copyright 2006-2012 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)
