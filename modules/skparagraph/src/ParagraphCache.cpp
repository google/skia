// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/ParagraphCache.h"

namespace skia {
namespace textlayout {

// Just the flutter input for now
// TODO: We don't really need to copy anything...
ParagraphCacheKey::ParagraphCacheKey(ParagraphImpl* paragraph)
        : fText(paragraph->fText)
        , fMapping() {
        paragraph->mapping().foreach([this](const char* ch, FontDescr* descr) {
        this->fMapping.set(ch - fText.c_str(), *descr);
    });
}

// TODO: copying clusters and runs for now (there are minor things changed on formatting)
ParagraphCacheValue::ParagraphCacheValue(ParagraphImpl* paragraph)
        : fKey(ParagraphCacheKey(paragraph))
        , fRuns(paragraph->fRuns)
        , fClusters(paragraph->fClusters) {
}

bool ParagraphCache::findParagraph(ParagraphImpl* paragraph) {

    SkAutoMutexExclusive lock(fParagraphMutex);
    if (this->count() == 0) {
        return false;
    }

    ParagraphCacheKey key(paragraph);
    auto found = this->find(key);
    if (found == nullptr) {
        fChecker(paragraph, "findParagraph", false);
        return false;
    }
    paragraph->fRuns.reset();
    paragraph->fRuns = found->fRuns;
    for (auto& run : paragraph->fRuns) {
        run.setMaster(paragraph);
    }

    paragraph->fClusters.reset();
    paragraph->fClusters = found->fClusters;
    for (auto& cluster : paragraph->fClusters) {
        cluster.setMaster(paragraph);
    }

    //fChecker(paragraph, "findParagraph", true);
    return true;
}

void ParagraphCache::addParagraph(ParagraphImpl* paragraph) {

    SkAutoMutexExclusive lock(fParagraphMutex);
    auto value = new ParagraphCacheValue(paragraph);
    this->add(value);
    SkDebugf("collisions: %d\n", this->countCollisions(value->fKey));
    fChecker(paragraph, "addParagraph", true);
}

const ParagraphCacheKey& LookupTrait::GetKey(const ParagraphCacheValue& paragraph) {
    return paragraph.fKey;
}

bool operator==(const ParagraphCacheKey& a, const ParagraphCacheKey& b) {
    if (a.fText.size() != b.fText.size()) {
        return false;
    }
    if (a.fMapping.count() != b.fMapping.count()) {
        return false;
    }
    if (a.fText != b.fText) {
        return false;
    }

    bool matches = true;
    a.fMapping.foreach([&](uint32_t index, FontDescr descr) {
        if (!matches) return;
        auto found = b.fMapping.find(index);
        if (found == nullptr || descr.fFont != found->fFont) {
            matches = false;
        }
    });

    return matches;
}

uint32_t LookupTrait::mix(uint32_t hash, uint32_t data) {
    hash += data;
    hash += (hash << 10);
    hash ^= (hash >> 6);
    return hash;
}

uint32_t LookupTrait::Hash(const ParagraphCacheKey& key) {
    uint32_t hash = 0;
    key.fMapping.foreach([&](uint32_t index, FontDescr descr){
        hash = mix(hash, SkGoodHash()(index));
        hash = mix(hash, SkGoodHash()(descr.fFont.getSize()));

        if (descr.fFont.getTypeface() != nullptr) {
            SkString name;
            descr.fFont.getTypeface()->getFamilyName(&name);
            hash = mix(hash, SkGoodHash()(name));
            hash = mix(hash, SkGoodHash()(descr.fFont.getTypeface()->fontStyle()));
        }
        SkDebugf("@%lu: hash=%lu\n", index, hash);
    });
    //hash = mix(hash, SkOpts::hash_fn(&key.fMapping, sizeof(key.fMapping), 0));
    hash = mix(hash, SkGoodHash()(key.fText));
    SkDebugf("hash=%d\n", hash);
    return hash;
}

void ParagraphCache::printCache(const char* title) {

    SkDebugf("\n\n%s\n", title);
    SkTDynamicHash<ParagraphCacheValue, ParagraphCacheKey, LookupTrait>::Iter iter(this);
    while (!iter.done()) {
        ParagraphCacheValue* v = &*iter;
        const ParagraphCacheKey& k = LookupTrait::GetKey(*v);
        SkDebugf("key: '%s' runs(%d) clusters(%d)\n", k.fText.c_str(), v->fRuns.size(), v->fClusters.size());
        ++iter;
    }
}

void ParagraphCache::printKeyValue(const char* title, ParagraphImpl* paragraph, bool found) {
    SkDebugf("%s '%s' ", title, paragraph->text().data());
    paragraph->mapping().foreach([](const char* ch, FontDescr* descr){
        SkDebugf("%d ", descr->fFont.getTypeface() != nullptr ? descr->fFont.getTypeface()->uniqueID(): 0);
    });
    SkDebugf("\n");
}

}
}
