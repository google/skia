/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "picture_utils.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkString.h"

namespace sk_tools {
    void force_all_opaque(const SkBitmap& bitmap) {
        SkASSERT(NULL == bitmap.getTexture());
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
        if (NULL == str) {
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

} // namespace sk_tools
