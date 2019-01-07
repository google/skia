/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXPSDocument_DEFINED
#define SkXPSDocument_DEFINED

#include "SkTypes.h"

#ifdef SK_BUILD_FOR_WIN

#include "SkDocument.h"

#include <memory>

struct IXpsOMObjectFactory;

namespace SkXPS {

#ifdef SK_SUPPORT_LEGACY_REFCNT_DOCUMENT
SK_API sk_sp<SkDocument> MakeDocument(SkWStream* stream,
                                      IXpsOMObjectFactory* xpsFactory,
                                      SkScalar dpi = SK_ScalarDefaultRasterDPI);
#else
SK_API std::unique_ptr<SkDocument> MakeDocument(SkWStream* stream,
                                                IXpsOMObjectFactory* xpsFactory,
                                                SkScalar dpi = SK_ScalarDefaultRasterDPI);
#endif

}  // namespace SkXPS
#endif  // SK_BUILD_FOR_WIN
#endif  // SkXPSDocument_DEFINED
