// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/ParagraphCache.h"

namespace skia {
namespace textlayout {

// Just the flutter input for now
// TODO: We don't really need to copy anything...
ParagraphCacheKey::ParagraphCacheKey(ParagraphImpl* paragraph)
        : fParaStyle(paragraph->paragraphStyle())
        , fText(paragraph->fText)
        , fMapping() {
        paragraph->mapping().foreach([this](const char* ch, FontDescr* descr) {
        this->fMapping.set(ch - fText.c_str(), *descr);
    });
}

// TODO: copying clusters and runs for now (there are minor things changed on formatting)
ParagraphCacheValue::ParagraphCacheValue(ParagraphImpl* paragraph)
        : fKey(ParagraphCacheKey(paragraph))
        , fRuns()
        , fClusters() {

    std::vector<std::pair<size_t, size_t>> updates;
    for (auto& run : paragraph->fRuns) {
        updates.emplace_back(run.clusters().begin() - paragraph->fClusters.begin(), run.clusters().size());
        fRuns.emplace_back(Run::correctRun(&run, fKey.fText.c_str() - paragraph->fText.c_str()));
    }

    for (auto& cluster: paragraph->fClusters) {
        fClusters.emplace_back(Cluster::correctCluster(
                &cluster,
                fRuns.begin() - paragraph->fRuns.begin(),
                fKey.fText.c_str() - paragraph->fText.c_str()));
    }

    for (size_t i = 0; i < fRuns.size(); ++i) {
        SkSpan<Cluster> span(fClusters.begin() + updates[i].first, updates[i].second);
        fRuns[i].setClusters(span);
    }
}

bool ParagraphCache::findParagraph(ParagraphImpl* paragraph) {

    SkAutoMutexExclusive lock(fParagraphMutex);
    if (this->count() == 0) {
        return false;
    }

    ParagraphCacheKey key(paragraph);
    auto found = this->find(key);
    if (found == nullptr) {
        fChecker("findParagraph", false);
        return false;
    }
    paragraph->fRuns.reset();
    std::vector<std::pair<size_t, size_t>> updates;
    for (auto& run : found->fRuns) {
        updates.emplace_back(run.clusters().begin() - found->fClusters.begin(), run.clusters().size());
        paragraph->fRuns.push_back(
                Run::correctRun(&run, paragraph->fText.c_str() - found->fKey.fText.c_str()));
    }
    paragraph->fClusters.reset();
    for (size_t i = 0; i < found->fClusters.size(); ++i) {
        paragraph->fClusters.emplace_back(Cluster::correctCluster(
                &found->fClusters[i],
                paragraph->fRuns.begin() - found->fRuns.begin(),
                paragraph->fText.c_str() - found->fKey.fText.c_str()));
    }

    for (size_t i = 0; i < paragraph->fRuns.size(); ++i) {
        SkSpan<Cluster> span(paragraph->fClusters.begin() + updates[i].first, updates[i].second);
        paragraph->fRuns[i].setClusters(span);
    }
    fChecker("findParagraph", true);
    return true;
}

void ParagraphCache::addParagraph(ParagraphImpl* paragraph) {

    SkAutoMutexExclusive lock(fParagraphMutex);
    this->add(new ParagraphCacheValue(paragraph));
    fChecker("addParagraph", true);
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

    bool matches = false;
    a.fMapping.foreach([&](size_t index, FontDescr descr) {
        if (matches) return;
        auto found = b.fMapping.find(index);
        if (found != nullptr && descr.fFont == found->fFont) {
            matches = true;
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
    uint32_t hash = 0;//SkGoodHash()(key.fParaStyle);
    key.fMapping.foreach([&](size_t index, FontDescr descr){
            hash = mix(hash, SkGoodHash()(index));
            hash = mix(hash, SkGoodHash()(descr.fFont));
    });
    //hash = mix(hash, SkOpts::hash_fn(&key.fMapping, sizeof(key.fMapping), 0));
    hash = mix(hash, SkGoodHash()(key.fText));
    return hash;
}

void ParagraphCache::printCache(const char* title) {

    SkDebugf("\n\n%s\n", title);
    SkTDynamicHash<ParagraphCacheValue, ParagraphCacheKey, LookupTrait>::Iter iter(this);
    while (!iter.done()) {
        ParagraphCacheValue* v = &*iter;
        const ParagraphCacheKey& k = LookupTrait::GetKey(*v);
        SkDebugf("key: '%s'\n", k.fText.c_str());
        k.fMapping.foreach([&](size_t index, FontDescr descr) {
            SkString name("<null>");
            descr.fFont.getTypeface()->getFamilyName(&name);
            SkDebugf("@%d: %s[%f]\n", index, name.c_str(), descr.fFont.getSize());
        });
        SkDebugf("value: runs(%d) clusters(%d)\n", v->fRuns.size(), v->fClusters.size());
        for (auto& run : v->fRuns) {
            SkDebugf("%s: clusters(%d), positions(%d)\n", run.text().begin(), run.clusters().size(), run.positions().size());
        }
        ++iter;
    }
}
}
}
