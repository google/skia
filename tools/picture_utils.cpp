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

static bool is_path_seperator(const char chr) {
#if defined(SK_BUILD_FOR_WIN)
    return chr == '\\' || chr == '/';
#else
    return chr == '/';
#endif
}

namespace sk_tools {
    void force_all_opaque(const SkBitmap& bitmap) {
        SkASSERT(NULL == bitmap.getTexture());
        SkASSERT(SkBitmap::kARGB_8888_Config == bitmap.config());
        if (NULL != bitmap.getTexture() || SkBitmap::kARGB_8888_Config == bitmap.config()) {
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

    void make_filepath(SkString* path, const SkString& dir, const SkString& name) {
        size_t len = dir.size();
        path->set(dir);
        if (0 < len  && '/' != dir[len - 1]) {
            path->append("/");
        }
        path->append(name);
    }

    void get_basename(SkString* basename, const SkString& path) {
        if (path.size() == 0) {
            basename->reset();
            return;
        }

        size_t end = path.size() - 1;

        // Paths pointing to directories often have a trailing slash,
        // we remove it so the name is not empty
        if (is_path_seperator(path[end])) {
            if (end == 0) {
                basename->reset();
                return;
            }

            end -= 1;
        }

        size_t i = end;
        do {
            --i;
            if (is_path_seperator(path[i])) {
                  const char* basenameStart = path.c_str() + i + 1;
                  size_t basenameLength = end - i;
                  basename->set(basenameStart, basenameLength);
                  return;
            }
        } while (i > 0);

        basename->set(path.c_str(), end + 1);
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
            partialPath = SkOSPath::SkPathJoin(dirPath.c_str(), subdirOrNull);
            sk_mkdir(partialPath.c_str());
        } else {
            partialPath.set(dirPath);
        }
        SkString fullPath = SkOSPath::SkPathJoin(partialPath.c_str(), baseName.c_str());
        if (SkImageEncoder::EncodeFile(fullPath.c_str(), bm, SkImageEncoder::kPNG_Type, 100)) {
            return true;
        } else {
            SkDebugf("Failed to write the bitmap to %s.\n", fullPath.c_str());
            return false;
        }
    }

} // namespace sk_tools
