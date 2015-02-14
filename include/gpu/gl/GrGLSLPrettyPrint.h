/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrGLSLPrettyPrint_DEFINED
#define GrGLSLPrettyPrint_DEFINED

#include "SkString.h"

namespace GrGLSLPrettyPrint {
    SkString PrettyPrintGLSL(const SkString& input, bool countlines);
};

#endif /* GRGLPRETTYPRINTSL_H_ */
