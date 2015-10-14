#! /usr/bin/env python
# Build Python extension with configuration file input
#
#  Copyright (C) 2006  Peter Johnson
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
from os.path import basename, join, exists

def ReadSetup(filename):
    """ReadSetup goes through filename and parses out the values stored
    in the file.  Values need to be stored in a
    \"key=value format\""""
    return dict(line.split('=', 1) for line in open(filename))

def ParseCPPFlags(flags):
    """parse the CPPFlags macro"""
    incl_dir = [x[2:] for x in flags.split() if x.startswith("-I")]
    cppflags = [x for x in flags.split() if not x.startswith("-I")]
    cppflags.append("-DYASM_LIB_INTERNAL")
    cppflags.append("-DYASM_BC_INTERNAL")
    cppflags.append("-DYASM_EXPR_INTERNAL")
    return (incl_dir, cppflags)

def ParseSources(src, srcdir):
    """parse the Sources macro"""
    # do the dance of detecting if the source file is in the current
    # directory, and if it's not, prepend srcdir
    sources = []
    for tok in src.split():
        if tok.endswith(".c"):
            fn = tok
        else:
            continue
        if not exists(fn):
            fn = join(srcdir, fn)
        sources.append(fn)

    return sources

def RunSetup(incldir, cppflags, sources):
    setup(
          name='yasm',
          version='0.0',
          description='Python bindings for Yasm',
          author='Michael Urman, Peter Johnson',
          url='http://www.tortall.net/projects/yasm',
          ext_modules=[
                       Extension('yasm',
                                 sources=sources,
                                 extra_compile_args=cppflags,
                                 include_dirs=incldir,
                       ),
                      ],
          cmdclass = dict(build_ext=build_ext),
         )

if __name__ == "__main__":
    opts = ReadSetup("python-setup.txt")
    incldir, cppflags = ParseCPPFlags(opts["includes"])
    sources = ParseSources(opts["sources"], opts["srcdir"].strip())
    sources.append('yasm_python.c')
    if opts["gcc"].strip() == "yes":
        cppflags.append('-w')
    RunSetup(incldir, cppflags, sources)

