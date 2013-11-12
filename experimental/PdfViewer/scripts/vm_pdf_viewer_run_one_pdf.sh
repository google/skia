#!/bin/bash

# Copyright 2013 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DIR=`dirname "$1"`

out/Debug/pdfviewer -r $1 -w $DIR/new -n

