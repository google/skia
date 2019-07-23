// Copyright 2019 Google LLC.
#ifndef ParagraphCache_DEFINED
#define ParagraphCache_DEFINED

#include <src/core/SkSpan.h>
#include <src/core/SkTDynamicHash.h>
#include "src/core/SkLRUCache.h"
#include "include/core/SkFont.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/src/FontResolver.h"
#include "include/core/SkPicture.h"

#define PARAGRAPH_CACHE_STATS

namespace skia {
namespace textlayout {

struct Measurement {
    SkScalar fAlphabeticBaseline;
    SkScalar fIdeographicBaseline;
    SkScalar fHeight;
    SkScalar fWidth;
    SkScalar fMaxIntrinsicWidth;
    SkScalar fMinIntrinsicWidth;
};

enum InternalState {
  kUnknown = 0,
  kShaped = 1,
  kClusterized = 2,
  kMarked = 3,
  kLineBroken = 4,
  kFormatted = 5,
  kDrawn = 6
};

class ParagraphImpl;
class ParagraphCacheKey {
public:
    ParagraphCacheKey(ParagraphImpl* paragraph);

    SkString fText;
    SkTArray<FontDescr> fFontSwitches;
    SkTArray<Block, true> fTextStyles;
    ParagraphStyle fParagraphStyle;
};

class ParagraphCacheValue {
public:;
    ParagraphCacheValue(ParagraphImpl* paragraph);

    // Input == key
    ParagraphCacheKey fKey;

    // Shaped results:
    InternalState fInternalState;
    SkTArray<Run> fRuns;
    SkTArray<Cluster, true> fClusters;
    SkTArray<RunShifts, true> fRunShifts;
};

bool operator==(const ParagraphCacheKey& a, const ParagraphCacheKey& b);

class ParagraphCache {
public:
    ParagraphCache();
    ~ParagraphCache();

    void abandon();
    void reset();
    bool updateParagraph(ParagraphImpl* paragraph);
    bool findParagraph(ParagraphImpl* paragraph);

    // For testing
    void setChecker(std::function<void(ParagraphImpl* impl, const char*, bool)> checker) {
        fChecker = std::move(checker);
    }

    void printStatistics();

 private:
     mutable SkMutex fParagraphMutex;
     std::function<void(ParagraphImpl* impl, const char*, bool)> fChecker;

    static const int kMaxEntries = 128;
    struct Entry;

    struct KeyHash {
        uint32_t mix(uint32_t hash, uint32_t data) const {
            hash += data;
            hash += (hash << 10);
            hash ^= (hash >> 6);
            return hash;
        }
        uint32_t operator()(const ParagraphCacheKey& key) const {
            uint32_t hash = 0;
            for (auto& fd : key.fFontSwitches) {
                hash = mix(hash, SkGoodHash()(fd.fStart));
                hash = mix(hash, SkGoodHash()(fd.fFont.getSize()));

                if (fd.fFont.getTypeface() != nullptr) {
                    SkString name;
                    fd.fFont.getTypeface()->getFamilyName(&name);
                    hash = mix(hash, SkGoodHash()(name));
                    hash = mix(hash, SkGoodHash()(fd.fFont.getTypeface()->fontStyle()));
                }
            }
            for (auto& ts : key.fTextStyles) {
                hash = mix(hash, SkGoodHash()(ts.fStyle.getLetterSpacing()));
                hash = mix(hash, SkGoodHash()(ts.fStyle.getWordSpacing()));
            }
            hash = mix(hash, SkGoodHash()(key.fText));
            return hash;
        }
    };

    SkLRUCache<ParagraphCacheKey, std::unique_ptr<Entry>, KeyHash> fMap;

#ifdef PARAGRAPH_CACHE_STATS
        int fTotalRequests;
        int fCacheMisses;
        int fHashMisses; // cache hit but hash table missed
#endif
};

}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphCache_DEFINED
