KWIML - The Kitware Information Macro Library

KWIML provides header files that use preprocessor tests to detect and
provide information about the compiler and its target architecture.  The
headers contain no configuration-time test results and thus may be
installed into an architecture-independent include directory.  This
makes them suitable for use in the public interface of any package.

This source tree is intended for distribution inside the source trees of
other packages.  In order to avoid name collisions among multiple
packages the KWIML headers are configured with a per-package prefix on
both the header locations and the macros they define.  See comments in
CMakeLists.txt for instructions to include KWIML inside another project.

The entire KWIML source tree is distributed under the OSI-approved
3-clause BSD License.  Files used only for build and test purposes
contain a copyright notice and reference Copyright.txt for details.
Headers meant for installation and distribution outside the source tree
come with full inlined copies of the copyright notice and license text.
This makes them suitable for distribution with any package under
compatible license terms.

The following components are provided.  See header comments for details:

  ABI.h = Fundamental type size and representation
  INT.h = Fixed-size integer types and format specifiers

The "test" subdirectory builds tests that verify correctness of the
information provided by each header.
