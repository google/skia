/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Just the flutter input for now
class SkParagraphCacheKey {

 public:
  SkParagraphCacheKey(
      sk_sp<SkFontCollection> collection,
      SkParagraphStyle paraStyle,
      SkTArray<SkBlock, true> textStyles,
      SkSpan<const char> utf8) : fHash(0) {
      fHash = mix(fHash, collection.computeHash());
      fHash = mix(fHash, paraStyle.computeHash());
      fHash = mix(fHash, computeHash(textStyles));
      fHash = mix(fHash, computeHash(utf8));
  }

    uint32_t hash() const { return fHash; }
 private:

  template<class T>
  uint32 computeHash(SkTArray<T, true> array) {
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

  inline uint32 mix(uint32 hash, uint32 data) {
      hash += data;
      hash += (hash << 10);
      hash ^= (hash >> 6);
      return hash;
  }
  
  uint32 fHash;
};

class SkParagraphCacheValue {

 public:
  SkParagraphCacheValue(
      sk_sp<SkFontCollection> collection,
      SkParagraphStyle paraStyle,
      SkTArray<SkBlock, true> textStyles,
      SkSpan<const char> utf8)
      : fKey(SkParagraphCacheKey(collection, paraStyle, textStyles, utf8))
      , fFontCollection(collection)
      , fParagraphStyle(paraStyle)
      , fTextStyles(textStyles)
      , fUtf8(utf8) { }

  static const SkParagraphCacheKey& GetKey(const SkParagraphCacheValue& value) { return fKey; }
  static uint32_t Hash(const SkParagraphCacheKey& key) { return fKey.hash(); }
  
 private:
  SkParagraphCacheKey fKey;

  sk_sp<SkFontCollection> fFontCollection;
  SkParagraphStyle fParagraphStyle;
  SkTArray<SkBlock, true> fTextStyles;
  SkSpan<const char> fUtf8;
};

class SkParagraphCache : public SkTDynamicHash<SkParagraphCacheValue, SkParagraphCacheKey>  {

 public:
  Hash() : INHERITED() {}

  // Promote protected methods to public for this test.
  int capacity() const { return this->INHERITED::capacity(); }
  int countCollisions(const int& key) const { return this->INHERITED::countCollisions(key); }

 private:
  typedef SkTDynamicHash<SkParagraphCacheValue, SkParagraphCacheKey> INHERITED;
};
