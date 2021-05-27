/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFallbacker.h"
#include "src/utils/SkUTF.h"

struct ConvertToUnichar {
    std::vector<SkUnichar> storage;
    const SkUnichar* uni;
    size_t count;

    ConvertToUnichar(const void* text, size_t byteLength, SkTextEncoding encoding) {
        switch (encoding) {
            case SkTextEncoding::kUTF8: {
                const char* ptr = (const char*)text;
                const char* end = ptr + byteLength;
                while (ptr < end) {
                    storage.push_back(SkUTF::NextUTF8(&ptr, end));
                }
                uni = storage.data();
                count = storage.size();
            } break;
            case SkTextEncoding::kUTF16: {
                const uint16_t* ptr = (const uint16_t*)text;
                const uint16_t* end = ptr + (byteLength >> 1);
                while (ptr < end) {
                    storage.push_back(SkUTF::NextUTF16(&ptr, end));
                }
                uni = storage.data();
                count = storage.size();
            } break;
            case SkTextEncoding::kUTF32:
                uni = (const SkUnichar*)text;
                count = SkToInt(byteLength >> 2);
                break;
            default:
                uni = nullptr;
                count = 0;
                break;
        }
    }
};

class Simple_fallbacker : public SkFallbacker {
public:
    Simple_fallbacker(SkSpan<SkTypeface*> array) {
        fFaces.resize(array.size());
        for (size_t i = 0; i < array.size(); ++i) {
            fFaces[i] = sk_ref_sp(array[i]);
        }
    }

    ~Simple_fallbacker() override = default;

    std::vector<Rec> resolve(const void* text, size_t byteLength,
                             SkTextEncoding encoding) const override {
        ConvertToUnichar src(text, byteLength, encoding);
        const SkUnichar* uni = src.uni;
        size_t count = src.count;

        std::vector<uint16_t> glyphStorage;
        glyphStorage.resize(count);
        uint16_t* glyphs = glyphStorage.data();

        // this is what we'll return
        std::vector<Rec> result;

        const size_t faceCount = fFaces.size();
        while (count > 0) {
            size_t i;
            for (i = 0; i < faceCount; ++i) {
                fFaces[i]->unicharsToGlyphs(uni, count, glyphs);

                size_t j;  // number of non-zero glyphs
                for (j = 0; j < count; ++j) {
                    if (glyphs[j] == 0) {
                        break;
                    }
                }
                if (j > 0) {
                    result.push_back({fFaces[i], j});
                    uni += j;
                    count -= j;
                    break;
                }
            }
            // did we fail to find any matching face?
            if (glyphs[0] == 0) {
                size_t j;  // number of zero glyphs
                for (j = 1; j < count; ++j) {
                    if (glyphs[j] != 0) {
                        break;
                    }
                }
                uni += j;
                count -= j;
                result.push_back({nullptr, j});
            }
        }

        // rec.textBytes is really a count at this point
        // Time to turn those "counts" back into actual textBytes
        uni = src.uni;

        for (Rec& rec : result) {
            SkDEBUGCODE(count += rec.textBytes;)
            size_t actualBytes = 0;
            switch (encoding) {
                case SkTextEncoding::kUTF8:
                    for (size_t i = 0; i < rec.textBytes; ++i) {
                        actualBytes += SkUTF::ToUTF8(uni[i]);
                    }
                    break;
                case SkTextEncoding::kUTF16:
                    for (size_t i = 0; i < rec.textBytes; ++i) {
                        actualBytes += SkUTF::ToUTF16(uni[i]) << 1;
                    }
                    break;
                case SkTextEncoding::kUTF32:
                    actualBytes = rec.textBytes << 2;
                    break;
                default: break;
            }
            rec.textBytes = actualBytes;
        }
        SkASSERT(count == src.count);

        return result;
    }

private:
    std::vector<sk_sp<SkTypeface>> fFaces;
};

sk_sp<SkFallbacker> SkFallbacker::MakeSimpleOrdered(SkSpan<SkTypeface*> array) {
    if (array.size() == 0) {
        return nullptr;
    }
    return sk_sp<SkFallbacker>(new Simple_fallbacker(array));
}
