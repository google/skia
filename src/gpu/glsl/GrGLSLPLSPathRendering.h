/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define GR_GL_PLS_DSTCOLOR_NAME  "pls.dstColor"
#define GR_GL_PLS_PATH_DATA_DECL "__pixel_localEXT PLSData {\n"\
                                 "    layout(rgba8i) ivec4 windings;\n"\
                                 "    layout(rgba8) vec4 dstColor;\n"\
                                 "} pls;\n"
