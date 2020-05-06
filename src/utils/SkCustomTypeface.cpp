/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"

class SkUserTypeface : public SkTypeface {
    friend class SkCustomTypefaceBuilder;
    friend class SkUserScalerContext;

    SkUserTypeface(int count)
        : SkTypeface(SkFontStyle())
        , fGlyphCount(count)
    {}

    const int fGlyphCount;
    std::vector<SkPath> fPaths;
    std::vector<float>  fAdvances;

protected:
    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                           const SkDescriptor* desc) const override;
    void onFilterRec(SkScalerContextRec* rec) const override;
    void getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override;

    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;

    void onGetFamilyName(SkString* familyName) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;

    std::unique_ptr<SkStreamAsset> onOpenStream(int*) const override;

    // trivial

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        return sk_ref_sp(this);
    }
    int onCountGlyphs() const override { return fGlyphCount; }
    int onGetUPEM() const override { return 2048; /* ?? */ }

    // noops

    void getPostScriptGlyphNames(SkString*) const override {}
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate[],
                                     int) const override { return 0; }
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis[],
                                       int) const override { return 0; }
    int onGetTableTags(SkFontTableTag tags[]) const override { return 0; }
    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override { return 0; }
};

SkCustomTypefaceBuilder::SkCustomTypefaceBuilder(int numGlyphs) : fGlyphCount(numGlyphs) {
    fAdvances.resize(numGlyphs);
    fPaths.resize(numGlyphs);
}

void SkCustomTypefaceBuilder::setGlyph(SkGlyphID index, float advance, const SkPath& path) {
    if (index >= (unsigned)fGlyphCount) {
        return;
    }
    fAdvances[index] = advance;
    fPaths[index]    = path;
}

sk_sp<SkTypeface> SkCustomTypefaceBuilder::detach() {
    SkUserTypeface* tf = new SkUserTypeface(fGlyphCount);
    tf->fAdvances = std::move(fAdvances);
    tf->fPaths    = std::move(fPaths);

    return sk_sp<SkTypeface>(tf);
}

/////////////

#include "src/core/SkScalerContext.h"

void SkUserTypeface::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkFontHinting::kNone);
}

void SkUserTypeface::getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const {
    for (int gid = 0; gid < fGlyphCount; ++gid) {
        glyphToUnicode[gid] = 0;
    }
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkUserTypeface::onGetAdvancedMetrics() const {
    return nullptr;
}

void SkUserTypeface::onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
    *isLocal = true;
}

void SkUserTypeface::onCharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const {
    for (int i = 0; i < count; ++i) {
        glyphs[i] = 0;
    }
}

void SkUserTypeface::onGetFamilyName(SkString* familyName) const {
    *familyName = "";
}

SkTypeface::LocalizedStrings* SkUserTypeface::onCreateFamilyNameIterator() const {
    return nullptr;
}

std::unique_ptr<SkStreamAsset> SkUserTypeface::onOpenStream(int* ttcIndex) const {
    SkDynamicMemoryWStream wstream;

    constexpr size_t kHeaderSize = 16;
    const char header[] = "SkUserTypeface00";
    static_assert(sizeof(header) == 1 + kHeaderSize, "need header to be 16 bytes");
    wstream.write(header, kHeaderSize);

    SkASSERT(fAdvances.size() == (unsigned)fGlyphCount);
    SkASSERT(fPaths.size() == (unsigned)fGlyphCount);

    wstream.write32(fGlyphCount);
    wstream.write(fAdvances.data(), fGlyphCount * sizeof(float));
    for (const auto& p : fPaths) {
        auto data = p.serialize();
        SkASSERT(SkIsAlign4(data->size()));
        wstream.write(data->data(), data->size());
    }
//    SkDebugf("%d glyphs, %d bytes\n", fGlyphCount, wstream.bytesWritten());
    return wstream.detachAsStream();
}

//////////////

#include "src/core/SkScalerContext.h"

class SkUserScalerContext : public SkScalerContext {
public:
    SkUserScalerContext(sk_sp<SkUserTypeface>           face,
                        const SkScalerContextEffects& effects,
                        const SkDescriptor*           desc)
            : SkScalerContext(std::move(face), effects, desc) {
        fRec.getSingleMatrix(&fMatrix);
        this->forceGenerateImageFromPath();
    }

    const SkUserTypeface* userTF() const {
        return static_cast<SkUserTypeface*>(this->getTypeface());
    }

protected:
    unsigned generateGlyphCount() override {
        return this->userTF()->fGlyphCount;
    }

    bool generateAdvance(SkGlyph* glyph) override {
        const SkUserTypeface* tf = this->userTF();
        auto advance = fMatrix.mapXY(tf->fAdvances[glyph->getGlyphID()], 0);

        glyph->fAdvanceX = advance.fX;
        glyph->fAdvanceY = advance.fY;
        return true;
    }

    void generateMetrics(SkGlyph* glyph) override {
        glyph->zeroMetrics();
        this->generateAdvance(glyph);
        // Always generates from paths, so SkScalerContext::getMetrics will figure the bounds.
    }

    void generateImage(const SkGlyph&) override { SK_ABORT("Should have generated from path."); }

    bool generatePath(SkGlyphID glyph, SkPath* path) override {
        this->userTF()->fPaths[glyph].transform(fMatrix, path);
        return true;
    }

    void generateFontMetrics(SkFontMetrics* metrics) override {
        // TODO
    }

private:
    SkMatrix fMatrix;
};

SkScalerContext* SkUserTypeface::onCreateScalerContext(const SkScalerContextEffects& effects,
                                                       const SkDescriptor*           desc) const {
    return new SkUserScalerContext(sk_ref_sp(const_cast<SkUserTypeface*>(this)), effects, desc);
}
