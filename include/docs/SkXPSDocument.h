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

    /** Clients can provide a way to encode png.
     *
     *  If `pngEncoder` is null and `SK_DISABLE_LEGACY_XPS_FACTORIES` isn't set
     *  then this will be (temporarily, until the legacy mode is removed)
     *  filled based on presence of `SK_CODEC_ENCODES_PNG_WITH_RUST` and/or
     *  `SK_CODEC_ENCODES_PNG_WITH_LIBPNG`.
     */
    EncodePngCallback pngEncoder = nullptr;

    /** Skia's XPS support depends on having a png encoder registered to handle
     *  XPS image brushes. OTOH, it will technically work on some XPS documents
     *  with a null `pngEncoder`. If clients will be creating XPS documents
     *  that don't need this, they should set `allowNoPngs` to true to avoid an
     *  internal assert from firing.
     */
    bool allowNoPngs = false;
};

SK_API sk_sp<SkDocument> MakeDocument(SkWStream* stream,
                                      IXpsOMObjectFactory* xpsFactory,
                                      Options opts);

#if !defined(SK_DISABLE_LEGACY_XPS_FACTORIES)

inline sk_sp<SkDocument> MakeDocument(SkWStream* stream, IXpsOMObjectFactory* xpsFactory) {
    Options opts;
    return MakeDocument(stream, xpsFactory, opts);
}

inline sk_sp<SkDocument> MakeDocument(SkWStream* stream,
                                      IXpsOMObjectFactory* xpsFactory,
                                      float dpi) {
    Options opts;
    opts.dpi = dpi;
    return MakeDocument(stream, xpsFactory, opts);
}

#endif  // !defined(SK_DISABLE_LEGACY_XPS_FACTORIES)

}  // namespace SkXPS
#endif  // SK_BUILD_FOR_WIN
#endif  // SkXPSDocument_DEFINED
