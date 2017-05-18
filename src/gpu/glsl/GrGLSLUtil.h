/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrGLSLUtil_DEFINED
#define GrGLSLUtil_DEFINED

class SkMatrix;

/**
 * Helper for converting SkMatrix to a column-major float array. We assume that all GLSL backends
 * use a column major representation for matrices.
 */
template<int MatrixSize> void GrGLSLGetMatrix(float* dest, const SkMatrix& src);

#endif
