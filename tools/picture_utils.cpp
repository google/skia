/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "picture_utils.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkHalf.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPM4fPriv.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkRasterPipeline.h"

#include "sk_tool_utils.h"

namespace sk_tools {
    void force_all_opaque(const SkBitmap& bitmap) {
        SkASSERT(kN32_SkColorType == bitmap.colorType());
        if (kN32_SkColorType == bitmap.colorType()) {
            return;
        }

        for (int y = 0; y < bitmap.height(); y++) {
            for (int x = 0; x < bitmap.width(); x++) {
                *bitmap.getAddr32(x, y) |= (SK_A32_MASK << SK_A32_SHIFT);
            }
        }
    }

    void replace_char(SkString* str, const char oldChar, const char newChar) {
        if (nullptr == str) {
            return;
        }
        for (size_t i = 0; i < str->size(); ++i) {
            if (oldChar == str->operator[](i)) {
                str->operator[](i) = newChar;
            }
        }
    }

    bool is_percentage(const char* const string) {
        SkString skString(string);
        return skString.endsWith("%");
    }

    void setup_bitmap(SkBitmap* bitmap, int width, int height) {
        bitmap->allocN32Pixels(width, height);
        bitmap->eraseColor(SK_ColorTRANSPARENT);
    }

    bool write_bitmap_to_disk(const SkBitmap& bm, const SkString& dirPath,
                              const char *subdirOrNull, const SkString& baseName) {
        SkString partialPath;
        if (subdirOrNull) {
            partialPath = SkOSPath::Join(dirPath.c_str(), subdirOrNull);
            sk_mkdir(partialPath.c_str());
        } else {
            partialPath.set(dirPath);
        }
        SkString fullPath = SkOSPath::Join(partialPath.c_str(), baseName.c_str());
        if (sk_tool_utils::EncodeImageToFile(fullPath.c_str(), bm, SkEncodedImageFormat::kPNG, 100)) {
            return true;
        } else {
            SkDebugf("Failed to write the bitmap to %s.\n", fullPath.c_str());
            return false;
        }
    }

    sk_sp<SkData> encode_bitmap_for_png(SkBitmap bitmap) {
        const int w = bitmap.width(),
                  h = bitmap.height();

        // PNG wants unpremultiplied 8-bit RGBA pixels (16-bit could work fine too).
        // We leave the gamma of these bytes unspecified, to continue the status quo,
        // which we think generally is to interpret them as sRGB.

        SkAutoTMalloc<uint32_t> rgba(w*h);

        const void* src = bitmap.getPixels();
        uint32_t*   dst = rgba.get();

        SkRasterPipeline_<256> p;
        switch (bitmap.colorType()) {
            case  kRGBA_F16_SkColorType: p.append(SkRasterPipeline::load_f16,  &src); break;
            case kBGRA_8888_SkColorType: p.append(SkRasterPipeline::load_bgra, &src); break;
            case kRGBA_8888_SkColorType: p.append(SkRasterPipeline::load_8888, &src); break;
            case   kRGB_565_SkColorType: p.append(SkRasterPipeline::load_565,  &src); break;
            default: SkASSERT(false);  // DM doesn't support any other formats, does it?
        }
        if (bitmap.info().gammaCloseToSRGB()) {
            p.append_from_srgb(kUnpremul_SkAlphaType);
        }
        p.append(SkRasterPipeline::unpremul);
        p.append(SkRasterPipeline::clamp_0);
        p.append(SkRasterPipeline::clamp_1);
        if (bitmap.info().colorSpace()) {
            // We leave legacy modes as-is.  They're already sRGB encoded (kind of).
            p.append(SkRasterPipeline::to_srgb);
        }
        p.append(SkRasterPipeline::store_8888, &dst);

        auto run = p.compile();
        for (int y = 0; y < h; y++) {
            run(0,y, w);
            src = SkTAddOffset<const void>(src, bitmap.rowBytes());
            dst += w;
        }

        return SkData::MakeFromMalloc(rgba.release(), w*h*sizeof(uint32_t));
    }

} // namespace sk_tools
