           // Copyright 2019 Google LLC.
#ifndef ParagraphCache_DEFINED
#define ParagraphCache_DEFINED

#include "include/private/SkMutex.h"
#include "src/core/SkLRUCache.h"
#include <functional>  // std::function

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
    SkScalar fLongestLine;
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
class ParagraphCacheKey;
class ParagraphCacheValue;

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
    void turnOn(bool value) { fCacheIsOn = value; }
    int count() { return fLRUCacheMap.count(); }

 private:

    struct Entry;
    void updateFrom(const ParagraphImpl* paragraph, Entry* entry);
    void updateTo(ParagraphImpl* paragraph, const Entry* entry);

     mutable SkMutex fParagraphMutex;
     std::function<void(ParagraphImpl* impl, const char*, bool)> fChecker;

    static const int kMaxEntries = 128;

    struct KeyHash {
        uint32_t mix(uint32_t hash, uint32_t data) const;
        uint32_t operator()(const ParagraphCacheKey& key) const;
    };

    SkLRUCache<ParagraphCacheKey, std::unique_ptr<Entry>, KeyHash> fLRUCacheMap;
    bool fCacheIsOn;

#ifdef PARAGRAPH_CACHE_STATS
    int fTotalRequests;
    int fCacheMisses;
    int fHashMisses; // cache hit but hash table missed
#endif
};

}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphCache_DEFINED
