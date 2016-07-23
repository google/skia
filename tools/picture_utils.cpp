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
#include "SkPM4fPriv.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkString.h"

namespace sk_tools {
    void force_all_opaque(const SkBitmap& bitmap) {
        SkASSERT(nullptr == bitmap.getTexture());
        SkASSERT(kN32_SkColorType == bitmap.colorType());
        if (bitmap.getTexture() || kN32_SkColorType == bitmap.colorType()) {
            return;
        }

        SkAutoLockPixels lock(bitmap);
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
        if (SkImageEncoder::EncodeFile(fullPath.c_str(), bm, SkImageEncoder::kPNG_Type, 100)) {
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

        auto srgbColorSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
        if (bitmap. colorType() ==  kN32_SkColorType &&
            bitmap.colorSpace() == srgbColorSpace.get()) {
            // These are premul sRGB 8-bit pixels in SkPMColor order.
            // We want unpremul sRGB 8-bit pixels in RGBA order.  We'll get there via floats.
            bitmap.lockPixels();
            auto px = (const uint32_t*)bitmap.getPixels();
            if (!px) {
                return nullptr;
            }
            for (int i = 0; i < w*h; i++) {
                Sk4f fs = Sk4f_fromS32(px[i]);         // Convert up to linear floats.
#if defined(SK_PMCOLOR_IS_BGRA)
                fs = SkNx_shuffle<2,1,0,3>(fs);        // Shuffle to RGBA, if not there already.
#endif
                float invA = 1.0f / fs[3];
                fs = fs * Sk4f(invA, invA, invA, 1);   // Unpremultiply.
                rgba[i] = Sk4f_toS32(fs);              // Pack down to sRGB bytes.
            }

        } else if (bitmap.colorType() == kRGBA_F16_SkColorType) {
            // These are premul linear half-float pixels in RGBA order.
            // We want unpremul sRGB 8-bit pixels in RGBA order.  We'll get there via floats.
            bitmap.lockPixels();
            auto px = (const uint64_t*)bitmap.getPixels();
            if (!px) {
                return nullptr;
            }
            for (int i = 0; i < w*h; i++) {
                // Convert up to linear floats.
                Sk4f fs(SkHalfToFloat(static_cast<SkHalf>(px[i] >> (0 * 16))),
                        SkHalfToFloat(static_cast<SkHalf>(px[i] >> (1 * 16))),
                        SkHalfToFloat(static_cast<SkHalf>(px[i] >> (2 * 16))),
                        SkHalfToFloat(static_cast<SkHalf>(px[i] >> (3 * 16))));
                fs = Sk4f::Max(0.0f, Sk4f::Min(fs, 1.0f));  // Clamp
                float invA = 1.0f / fs[3];
                fs = fs * Sk4f(invA, invA, invA, 1);  // Unpremultiply.
                rgba[i] = Sk4f_toS32(fs);             // Pack down to sRGB bytes.
            }

        } else {
            // We "should" gamma correct in here but we don't.
            // We want Gold to show exactly what our clients are seeing, broken gamma.

            // Convert smaller formats up to premul linear 8-bit (in SkPMColor order).
            if (bitmap.colorType() != kN32_SkColorType) {
                SkBitmap n32;
                if (!bitmap.copyTo(&n32, kN32_SkColorType)) {
                    return nullptr;
                }
                bitmap = n32;
            }

            // Convert premul linear 8-bit to unpremul linear 8-bit RGBA.
            if (!bitmap.readPixels(SkImageInfo::Make(w,h, kRGBA_8888_SkColorType,
                                                     kUnpremul_SkAlphaType),
                                   rgba, 4*w, 0,0)) {
                return nullptr;
            }
        }

        return SkData::MakeFromMalloc(rgba.release(), w*h*sizeof(uint32_t));
    }

} // namespace sk_tools
