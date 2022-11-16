# Copyright (c) 2022 Google LLC. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This contains additional Skia helpers for use with the CodeLLDB debugger extension to VSCode
# https://github.com/vadimcn/vscode-lldb

# To use these features, run the following command at the (lldb) prompt within VSCode:
#
#      command script import (your-skia-local-path)/platform_tools/debugging/lldb/skiavis.py
#
# This can be automatically enabled at the start of every debugging session by adding that command
# to the "initCommands" of your launch configuration, as explained here:
# https://github.com/vadimcn/vscode-lldb/wiki/Data-visualization
#
# From my testing, the instance of LLDB that runs within CodeLLDB does not invoke ~/.lldbinit,
# so adding the command there will not work. (In addition, this script uses modules that won't
# be present in command-line LLDB).

import lldb
import debugger

# Helper function to draw an SkPath as an SVG document within VSCode
# Call it with a string containing the path expression (eg, variable name):
#
#     ?/py skiavis.show_path('fPath')
#
def show_path(path):
    pathStr = debugger.evaluate('SkParsePath::ToSVGString(' + path +
                                ', SkParsePath::PathEncoding::Absolute)')
    svg = '<svg><path d="' + pathStr + '"/></svg>'
    debugger.display_html(svg)
