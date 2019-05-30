/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ParagraphCache_DEFINED
#define ParagraphCache_DEFINED

namespace skia {
namespace textlayout {

// Just the flutter input for now
class ParagraphCacheKey {
public:
    ParagraphCacheKey(sk_sp<SkFontCollection> collection,
                        SkParagraphStyle paraStyle,
                        SkTArray<SkBlock, true>
                                textStyles,
                        SkSpan<const char>
                                utf8)
            : fHash(0) {
        fHash = mix(fHash, collection.computeHash());
        fHash = mix(fHash, paraStyle.computeHash());
        fHash = mix(fHash, computeHash(textStyles));
        fHash = mix(fHash, computeHash(utf8));
    }

    uint32_t hash() const { return fHash; }

private:
    template <class T> uint32 computeHash(SkTArray<T, true> array) {
        uint32 hash = 0;
        for (auto& t : array) {
            hash = mix(hash, t.computeHash());
        }
        return hash;
    }

    uint32 computeHash(SkSpan<const char> text) {
        uint32 hash = mix(0, text.size());
        for (uint32 i = 0; i < text.size(); i += 2) {
            uint32 data = text[i] | text[i + 1] << 16;
            hash = mix(hash, data);
        }
        if (text.size() & 1) {
            uint32 data = text.back();
            hash = mix(hash, data);
        }
        return hash;
    }

    uint32 mix(uint32 hash, uint32 data) {
        hash += data;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        return hash;
    }

    uint32 fHash;
};

class ParagraphCacheValue {
public:
    ParagraphCacheValue(sk_sp<SkFontCollection> collection,
                          SkParagraphStyle paraStyle,
                          SkTArray<SkBlock, true>
                                  textStyles,
                          SkSpan<const char>
                                  utf8)
            : fKey(ParagraphCacheKey(collection, paraStyle, textStyles, utf8))
            , fFontCollection(collection)
            , fParagraphStyle(paraStyle)
            , fTextStyles(textStyles)
            , fUtf8(utf8) {}

    static const ParagraphCacheKey& GetKey(const ParagraphCacheValue& value) { return fKey; }
    static uint32_t Hash(const ParagraphCacheKey& key) { return fKey.hash(); }

private:
    ParagraphCacheKey fKey;

    sk_sp<SkFontCollection> fFontCollection;
    SkParagraphStyle fParagraphStyle;
    SkTArray<SkBlock, true> fTextStyles;
    SkSpan<const char> fUtf8;
};

class ParagraphCache : public SkTDynamicHash<ParagraphCacheValue, ParagraphCacheKey> {
public:
    Hash() : INHERITED() {}

    // Promote protected methods to public for this test.
    int capacity() const { return this->INHERITED::capacity(); }
    int countCollisions(const int& key) const { return this->INHERITED::countCollisions(key); }

private:
    typedef SkTDynamicHash<ParagraphCacheValue, ParagraphCacheKey> INHERITED;
};
}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphCache_DEFINED
