// Copyright 2019 Google LLC.
#ifndef ParagraphCache_DEFINED
#define ParagraphCache_DEFINED

#include "include/private/base/SkMutex.h"
#include <functional>  // std::function

#define PARAGRAPH_CACHE_STATS

namespace skia {
namespace textlayout {

class ParagraphImpl;

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
    void turnOn(bool value);
    int count();

    bool isPossiblyTextEditing(ParagraphImpl* paragraph);

 private:

    struct Entry;
    void updateFrom(const ParagraphImpl* paragraph, Entry* entry);
    void updateTo(ParagraphImpl* paragraph, const Entry* entry);

     mutable SkMutex fParagraphMutex;
     std::function<void(ParagraphImpl* impl, const char*, bool)> fChecker;

    struct Cache;
    std::unique_ptr<Cache> fCache;

};

}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphCache_DEFINED
