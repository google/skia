/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrUniformHandle_DEFINED
#define GrUniformHandle_DEFINED

namespace {
inline int handle_to_index(GrGLUniformManager::UniformHandle h) { return ~h; }
inline GrGLUniformManager::UniformHandle index_to_handle(int i) { return ~i; }
}

#endif
