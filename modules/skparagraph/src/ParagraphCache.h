// Copyright 2019 Google LLC.
#ifndef ParagraphCache_DEFINED
#define ParagraphCache_DEFINED

namespace skia {
namespace textlayout {

// Just the flutter input for now
class ParagraphCacheKey {
public:
    ParagraphCacheKey(SkParagraphStyle paraStyle,
                      SkTHashMap<const char*, std::pair<SkFont, SkScalar>> mapping,
                      SkSpan<const char> utf8)
            : fHash(0) {
        fHash = mix(fHash, paraStyle.computeHash());
        fHash = mix(fHash, computeHash(mapping));
        fHash = mix(fHash, computeHash(utf8));
    }

    uint32_t hash() const { return fHash; }

private:
    uint32 computeHash(SkTHashMap<const char*, std::pair<SkFont, SkScalar>> mapping) {
        uint32 hash = 0;
        mapping.forEach([&hash](const char* ch, std::pair<SkFont, SkScalar> font) {
            hash = mix(hash, t.computeHash());
        });
        for (auto& t : array) {
            hash = mix(hash, ch);
            hash = mix(hash, SkGoodHash(font.first));
            hash = mix(hash, SkGoodHash(font.second));
        }
        return hash;
    }

    uint32 computeHash(SkSpan<const char> text) {}

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
    ParagraphCacheValue(std::shared<SkParagraph> paragraph,
                        SkTHashMap<const char*,
                        std::pair<SkFont, SkScalar>> mapping)
        : fKey(ParagraphCacheKey(paragraph.getParagraphStyle(), mapping, paragraph.getText()))
        , fFontCollection(collection)
        , fParagraphStyle(paraStyle)
        , fTextStyles(textStyles)
        , fUtf8(utf8) {}

    static const ParagraphCacheKey& GetKey(const ParagraphCacheValue& value) { return fKey; }
    static uint32_t Hash(const ParagraphCacheKey& key) { return fKey.hash(); }

private:
    ParagraphCacheKey fKey;

    std::shared<SkParagraph> fParagraph;
    std::pair<SkFont, SkScalar>>fMapping;
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
