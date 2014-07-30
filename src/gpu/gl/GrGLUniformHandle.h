/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrUniformHandle_DEFINED
#define GrUniformHandle_DEFINED

inline GrGLProgramDataManager::UniformHandle GrGLProgramDataManager::UniformHandle::CreateFromUniformIndex(int index) {
    return GrGLProgramDataManager::UniformHandle(index);
}

#endif
