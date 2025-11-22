/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXPSDocument_DEFINED
#define SkXPSDocument_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_BUILD_FOR_WIN

#include "include/core/SkDocument.h"

struct IXpsOMObjectFactory;
class SKWStream;
class SkPixmap;

namespace SkXPS {

using EncodePngCallback = bool (*)(SkWStream* dst, const SkPixmap& src);

struct Options {
    float dpi = SK_ScalarDefaultRasterDPI;

    /** Clients can provide a way to encode png. */
    EncodePngCallback pngEncoder = nullptr;

    /** Skia's XPS support depends on having a png encoder registered to handle
     *  XPS image brushes. Clients that create XPS documents without images may
     *  set `allowNoPngs` to true to acknowledge this.
     */
    bool allowNoPngs = false;
};

SK_API sk_sp<SkDocument> MakeDocument(SkWStream* stream,
                                      IXpsOMObjectFactory* xpsFactory,
                                      Options opts);
}  // namespace SkXPS
#endif  // SK_BUILD_FOR_WIN
#endif  // SkXPSDocument_DEFINED
