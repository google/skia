/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFallbacker.h"

struct ConvertToUnichar {
    std::vector<SkUnichar> storage;
    const SkUnichar* uni;
    int count;

    ConvertToUnichar(const void* text, size_t byteLength, SkTextEncoding encoding) {
        switch (encoding) {
            case SkTextEncoding::kUTF8: {
                uni = fStorage.reset(byteLength);
                const char* ptr = (const char*)text;
                const char* end = ptr + byteLength;
                for (int i = 0; ptr < end; ++i) {
                    storage.push_back(SkUTF::NextUTF8(&ptr, end));
                }
                uni = storage.data();
                count = SkToInt(storage.size());
            } break;
            case SkTextEncoding::kUTF16: {
                const uint16_t* ptr = (const uint16_t*)text;
                const uint16_t* end = ptr + (byteLength >> 1);
                for (int i = 0; ptr < end; ++i) {
                    storage.push_back(SkUTF::NextUTF16(&ptr, end));
                }
                uni = storage.data();
                count = SkToInt(storage.size());
            } break;
            case SkTextEncoding::kUTF32:
                uni = (const SkUnichar*)text;
                count = SkToInt(byteLength >> 2);
                break;
            default:
                sk_throw();
        }
    }
};

class Simple_fallbacker : public SkFallbacker {
public:
    std::vector<Rec> resolve(const void* text, size_t byteLength,
                             SkTextEncoding encoding) const override {
        ConvertToUnichar src(text, byteLength, encoding);
        const SkUnichar* uni = src.uni;
        int count = src.count;

        std::vector<uint16_t> glyphStorage;
        glyphStorage.resize(count);
        uint16_t* glyphs = glyphStorage.data();

        while (count > 0) {
            for (auto tf : fTypefaces) {
                SkTypeface* found = nullptr;
                tf->unicharsToGlyphs(uni, count, glyphs);
                if (glyphs[0]) { // did we get even one hit?
                    found = tf;
                    break;
                }
            }
        }
        void unicharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const;

    }

    static sk_sp<SkFallbacker> Make(SkSpan<SkTypeface*> array) {

    }
};

class SkSimpleFallbacker {
public:
    static sk_sp<SkFallbacker> Make(SkSpan<SkTypeface*> inPreferredOrder);
};
