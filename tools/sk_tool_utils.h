/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_tool_utils_DEFINED
#define sk_tool_utils_DEFINED

#include "SkColor.h"
#include "SkImageEncoder.h"
#include "SkImageInfo.h"
#include "SkPixelSerializer.h"
#include "SkTypeface.h"

class SkBitmap;
class SkCanvas;
class SkPaint;
class SkShader;
class SkTestFont;
class SkTextBlobBuilder;

namespace sk_tool_utils {

    const char* colortype_name(SkColorType);

    /**
     * Sets the paint to use a platform-independent text renderer.
     */
    void set_portable_typeface(SkPaint* paint, const char* name = NULL,
                               SkTypeface::Style style = SkTypeface::kNormal);
    SkTypeface* create_portable_typeface(const char* name, SkTypeface::Style style);
    void report_used_chars();

    /**
     *  Call canvas->writePixels() by using the pixels from bitmap, but with an info that claims
     *  the pixels are colorType + alphaType
     */
    void write_pixels(SkCanvas*, const SkBitmap&, int x, int y, SkColorType, SkAlphaType);

    // private to sk_tool_utils
    SkTypeface* create_font(const char* name, SkTypeface::Style );
    SkTypeface* resource_font(const char* name, SkTypeface::Style );

    /** Returns a newly created CheckerboardShader. */
    SkShader* create_checkerboard_shader(SkColor c1, SkColor c2, int size);

    /** Draw a checkerboard pattern in the current canvas, restricted to
        the current clip, using SkXfermode::kSrc_Mode. */
    void draw_checkerboard(SkCanvas* canvas,
                           SkColor color1,
                           SkColor color2,
                           int size);

    /** A default checkerboard. */
    inline void draw_checkerboard(SkCanvas* canvas) {
        sk_tool_utils::draw_checkerboard(canvas, 0xFF999999, 0xFF666666, 8);
    }

    // Encodes to PNG, unless there is already encoded data, in which case that gets
    // used.
    class PngPixelSerializer : public SkPixelSerializer {
    public:
        bool onUseEncodedData(const void*, size_t) override { return true; }
        SkData* onEncodePixels(const SkImageInfo& info, const void* pixels,
                               size_t rowBytes) override {
            return SkImageEncoder::EncodeData(info, pixels, rowBytes,
                                              SkImageEncoder::kPNG_Type, 100);
        }
    };

    // A helper for inserting a drawtext call into a SkTextBlobBuilder
    void add_to_text_blob(SkTextBlobBuilder* builder, const char* text, const SkPaint& origPaint,
                          SkScalar x, SkScalar y);

}  // namespace sk_tool_utils

#endif  // sk_tool_utils_DEFINED
