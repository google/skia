/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/svg/SkSVGCanvas.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTo.h"
#include "src/svg/SkSVGDevice.h"
#include "src/xml/SkXMLWriter.h"

#include <utility>

#if !defined(SK_DISABLE_LEGACY_SVG_FACTORIES)
#if defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
#include "include/encode/SkPngRustEncoder.h"
#else
#include "include/encode/SkPngEncoder.h"
#endif  // SK_CODEC_ENCODES_PNG_WITH_RUST
#endif  // !defined(SK_DISABLE_LEGACY_SVG_FACTORIES)

std::unique_ptr<SkCanvas> SkSVGCanvas::Make(const SkRect& bounds, SkWStream* writer,
                                            Options opts) {
#if !defined(SK_DISABLE_LEGACY_SVG_FACTORIES)
    if (!opts.pngEncoder) {
        opts.pngEncoder = [](SkWStream* dst, const SkPixmap& src) {
#if defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
            return SkPngRustEncoder::Encode(dst, src, {});
#else
            return SkPngEncoder::Encode(dst, src, {});
#endif // defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
        };
    }
#else
    if (!opts.pngEncoder) {
        SK_ABORT("Must set a PNG encoder to make SVG canvas");
    }
#endif  // !defined(SK_DISABLE_LEGACY_SVG_FACTORIES)

    // TODO: pass full bounds to the device
    const auto size = bounds.roundOut().size();
    const auto xml_flags = (opts.flags & kNoPrettyXML_Flag)
        ? SkToU32(SkXMLStreamWriter::kNoPretty_Flag)
        : 0;

    auto svgDevice = SkSVGDevice::Make(size,
                                       std::make_unique<SkXMLStreamWriter>(writer, xml_flags),
                                       opts);

    return svgDevice ? std::make_unique<SkCanvas>(std::move(svgDevice))
                     : nullptr;
}

#if !defined(SK_DISABLE_LEGACY_SVG_FACTORIES)
std::unique_ptr<SkCanvas> SkSVGCanvas::Make(const SkRect& bounds,
                                            SkWStream* stream,
                                            uint32_t flags) {
    Options opts;
    opts.flags = static_cast<Flags>(flags);
    return Make(bounds, stream, opts);
}
#endif
