/*
 * Copyright 2010, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GLUOS_H_
#define GLUOS_H_

// This header provides the minimal definitions needed to compile the
// GLU tessellator sources.
#define GLAPIENTRY

typedef unsigned char GLboolean;
typedef double GLdouble;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef void GLvoid;

#define gluNewTess Sk_gluNewTess
#define gluDeleteTess Sk_gluDeleteTess
#define gluTessProperty Sk_gluTessProperty
#define gluGetTessProperty Sk_gluGetTessProperty
#define gluTessNormal Sk_gluTessNormal
#define gluTessCallback Sk_gluTessCallback
#define gluTessVertex Sk_gluTessVertex
#define gluTessBeginPolygon Sk_gluTessBeginPolygon
#define gluTessBeginContour Sk_gluTessBeginContour
#define gluTessEndContour Sk_gluTessEndContour
#define gluTessEndPolygon Sk_gluTessEndPolygon

#define __gl_noBeginData Sk__gl_noBeginData
#define __gl_noEdgeFlagData Sk__gl_noEdgeFlagData
#define __gl_noVertexData Sk__gl_noVertexData
#define __gl_noEndData Sk__gl_noEndData
#define __gl_noErrorData Sk__gl_noErrorData
#define __gl_noCombineData Sk__gl_noCombineData

#undef MIN
#undef MAX

#endif  // GLUOS_H_
