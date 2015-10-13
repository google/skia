#!/bin/sh
# Copyright (c) 2013 Luca Barbato
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom
# the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

if [ -d .git ]; then
    #  Retrieve the version from the last git tag.
    VER=`git describe --always | sed -e "s:v::"`
    if [ x"`git diff-index --name-only HEAD`" != x ]; then
        #  If the sources have been changed locally, add -dirty to the version.
        VER="${VER}-dirty"
    fi
elif [ -f .version ]; then
    #  If git is not available (e.g. when building from source package)
    #  we can extract the package version from .version file.
    VER=`< .version`
else
    #  The package version cannot be retrieved.
    VER="Unknown"
fi

printf '%s' "$VER"


