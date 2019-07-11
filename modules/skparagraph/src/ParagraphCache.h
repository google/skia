// Copyright 2019 Google LLC.
#ifndef ParagraphCache_DEFINED
#define ParagraphCache_DEFINED

#include <src/core/SkSpan.h>
#include <src/core/SkTDynamicHash.h>
#include "include/core/SkFont.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/src/FontResolver.h"

// TODO: we can cache shaped results or line broken results or formatted results or recorded picture...
// TODO: let's start from the first: the shaped results
namespace skia {
namespace textlayout {

class ParagraphImpl;
class ParagraphCacheKey {
public:
    ParagraphCacheKey(ParagraphImpl* paragraph);

    SkString fText;
    SkTArray<FontDescr> fFontSwitches;
    // TODO: Do not check for more than we have to (which is text styles with letter spacing and word spacing)
    SkTArray<TextBlock, true> fTextStyles;
};

class ParagraphCacheValue {
public:;
    ParagraphCacheValue(ParagraphImpl* paragraph);

    // Input == key
    ParagraphCacheKey fKey;

    // Shaped results:
    SkTArray<Run> fRuns;
    SkTArray<Cluster, true> fClusters;
};

bool operator==(const ParagraphCacheKey& a, const ParagraphCacheKey& b);

struct LookupTrait {
    static const ParagraphCacheKey& GetKey(const ParagraphCacheValue& paragraph);
    static uint32_t Hash(const ParagraphCacheKey& key);
    static uint32_t mix(uint32_t hash, uint32_t data);
};

// TODO: can we cache the key by the text?... So we do not need to do font resolution again
class ParagraphCache : public SkTDynamicHash<ParagraphCacheValue, ParagraphCacheKey, LookupTrait> {
public:

    ParagraphCache() : fChecker([](ParagraphImpl* impl, const char*, bool){ }){ }
    bool findParagraph(ParagraphImpl* paragraph);
    void addParagraph(ParagraphImpl* paragraph);

    // For testing

    void setChecker(std::function<void(ParagraphImpl* impl, const char*, bool)> checker) { fChecker = checker; }
    void printCache(const char* title);
    void printKeyValue(const char* title, ParagraphImpl* paragraph, bool found);

 private:
     mutable SkMutex fParagraphMutex;
     std::function<void(ParagraphImpl* impl, const char*, bool)> fChecker;
};

}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphCache_DEFINED
