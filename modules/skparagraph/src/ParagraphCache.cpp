// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/ParagraphCache.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

namespace skia {
namespace textlayout {

// Just the flutter input for now
// TODO: We don't really need to copy anything...
ParagraphCacheKey::ParagraphCacheKey(ParagraphImpl* paragraph)
        : fText(paragraph->fText)
        , fFontSwitches(paragraph->switches())
        , fTextStyles(paragraph->fTextStyles)
        , fParagraphStyle(paragraph->paragraphStyle()) { }

// TODO: copying clusters and runs for now (there are minor things changed on formatting)
ParagraphCacheValue::ParagraphCacheValue(ParagraphImpl* paragraph)
        : fKey(ParagraphCacheKey(paragraph))
        , fInternalState(paragraph->state())
        , fRuns(paragraph->fRuns)
        , fClusters(paragraph->fClusters)
        , fMeasurement(paragraph->measurement())
        , fPicture(paragraph->fPicture) { }

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

    paragraph->fLines.reset();
    for (auto& line : found->fLines) {
        paragraph->fLines.push_back(line);
        paragraph->fLines.back().setMaster(paragraph);
    }

    paragraph->fState = found->fInternalState;
    paragraph->setMeasurement(found->fMeasurement);

    paragraph->fOldWidth = found->fMeasurement.fWidth;
    paragraph->fOldHeight = found->fMeasurement.fHeight;

    paragraph->fPicture = found->fPicture;

    fChecker(paragraph, "findParagraph", true);
    return true;
}

void ParagraphCache::addParagraph(ParagraphImpl* paragraph) {

    SkAutoMutexExclusive lock(fParagraphMutex);
    auto value = new ParagraphCacheValue(paragraph);
    this->add(value);
    fChecker(paragraph, "addParagraph1", true);
}

void ParagraphCache::updateParagraph(ParagraphImpl* paragraph) {

    SkAutoMutexExclusive lock(fParagraphMutex);
    ParagraphCacheKey key(paragraph);
    auto found = this->find(key);
    if (found != nullptr) {
        found->fInternalState = paragraph->fState;
        found->fMeasurement = paragraph->measurement();
        found->fLines = paragraph->fLines;
        for (size_t i = 0; i < paragraph->fRuns.size(); ++i) {
            auto& run = paragraph->fRuns[i];
            if (run.fSpaced) {
                found->fRuns[i] = run;
            }
        }
        found->fPicture = paragraph->fPicture;
        fChecker(paragraph, "updateParagraph", true);
    } else {
        auto value = new ParagraphCacheValue(paragraph);
        this->add(value);
        fChecker(paragraph, "addParagraph2", true);
    }
}

const ParagraphCacheKey& LookupTrait::GetKey(const ParagraphCacheValue& paragraph) {
    return paragraph.fKey;
}

bool operator==(const ParagraphCacheKey& a, const ParagraphCacheKey& b) {
    if (a.fText.size() != b.fText.size()) {
        return false;
    }
    if (a.fFontSwitches.count() != b.fFontSwitches.count()) {
        return false;
    }
    if (a.fText != b.fText) {
        return false;
    }
    if (a.fTextStyles.size() != b.fTextStyles.size()) {
        return false;
    }

    if (a.fParagraphStyle.getMaxLines() != b.fParagraphStyle.getMaxLines()) {
        // TODO: this is too strong, but at least we will not lose lines
        return false;
    }

    for (size_t i = 0; i < a.fFontSwitches.size(); ++i) {
        auto& fda = a.fFontSwitches[i];
        auto& fdb = b.fFontSwitches[i];
        if (fda.fStart != fdb.fStart) {
            return false;
        }
        if (fda.fFont != fdb.fFont) {
            return false;
        }
    }

    for (size_t i = 0; i < a.fTextStyles.size(); ++i) {
        auto& tsa = a.fTextStyles[i];
        auto& tsb = b.fTextStyles[i];
        if (tsa.fStyle.getLetterSpacing() != tsb.fStyle.getLetterSpacing()) {
            return false;
        }
        if (tsa.fStyle.getWordSpacing() != tsb.fStyle.getWordSpacing()) {
            return false;
        }
        if (tsa.fRange.width() != tsb.fRange.width()) {
            return false;
        }
        if (tsa.fRange.start != tsb.fRange.start) {
            return false;
        }
    }

    return true;
}

uint32_t LookupTrait::mix(uint32_t hash, uint32_t data) {
    hash += data;
    hash += (hash << 10);
    hash ^= (hash >> 6);
    return hash;
}

uint32_t LookupTrait::Hash(const ParagraphCacheKey& key) {
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
    /*
    SkDebugf("%s '%s' ", title, paragraph->text().data());
    for (auto& fd : paragraph->switches()) {
        SkDebugf("%d ", fd.fFont.getTypeface() != nullptr ? fd.fFont.getTypeface()->uniqueID(): 0);
    };
    SkDebugf("\n");
     */
}

}
}
