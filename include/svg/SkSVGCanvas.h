/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGCanvas_DEFINED
#define SkSVGCanvas_DEFINED

#include "include/core/SkTypes.h"

#include <cstdint>
#include <memory>

class SkCanvas;
class SkWStream;
class SkPixmap;
struct SkRect;

class SK_API SkSVGCanvas {
public:
    enum Flags {
        kConvertTextToPaths_Flag   = 0x01, // emit text as <path>s
        kNoPrettyXML_Flag          = 0x02, // suppress newlines and tabs in output
        kRelativePathEncoding_Flag = 0x04, // use relative commands for path encoding
    };

    using EncodePngCallback = bool (*)(SkWStream* dst, const SkPixmap& src);

    struct Options {
        Flags flags = static_cast<Flags>(0x00);

        /** Clients can provide a way to encode png.
         *
         *  If `pngEncoder` is null and `SK_DISABLE_LEGACY_SVG_FACTORIES` isn't
         *  set then this will be (temporarily, until the legacy mode is
         *  removed) filled based on presence of
         *  `SK_CODEC_ENCODES_PNG_WITH_RUST` and/or
         *  `SK_CODEC_ENCODES_PNG_WITH_LIBPNG`.
         */
        EncodePngCallback pngEncoder = nullptr;
    };

    /**
     *  Returns a new canvas that will generate SVG commands from its draw calls, and send
     *  them to the provided stream. Ownership of the stream is not transfered, and it must
     *  remain valid for the lifetime of the returned canvas.
     *
     *  The canvas may buffer some drawing calls, so the output is not guaranteed to be valid
     *  or complete until the canvas instance is deleted.
     *
     *  The 'bounds' parameter defines an initial SVG viewport (viewBox attribute on the root
     *  SVG element).
     */
    static std::unique_ptr<SkCanvas> Make(const SkRect& bounds, SkWStream*, Options opts);
#if !defined(SK_DISABLE_LEGACY_SVG_FACTORIES)
    static std::unique_ptr<SkCanvas> Make(const SkRect& bounds, SkWStream*, uint32_t flags = 0);
#endif
};

#endif
