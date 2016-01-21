/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkValueKeys_DEFINED
#define SkValueKeys_DEFINED

namespace SkValueKeys {
    namespace ArithmeticXfermode {
        enum { kK0, kK1, kK2, kK3, kEnforcePMColor };
    }
    namespace LerpXfermode { enum { kScale }; }
    namespace PixelXorXfermode { enum { kOpColor }; }
    namespace ProcCoeffXfermode { enum { kMode }; }
}

#endif  // SkValueKeys_DEFINED
