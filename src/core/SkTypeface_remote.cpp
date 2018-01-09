/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDescriptor.h"
#include "SkPaint.h"
#include "SkPathEffect.h"
#include "SkMaskFilter.h"
#include "SkRasterizer.h"
#include "SkTypeface_remote.h"
#include "SkWriteBuffer.h"
#include <unordered_map>
#include <iostream>

// All the SkTypeface objects are SkTypefaceProxy objects.
class RemoteScalerContext {
public:
    virtual ~RemoteScalerContext() {}
    virtual void generateFontMetrics(
            const SkTypefaceProxy& tf,
            const SkScalerContextRec& rec,
            const SkScalerContextEffects& effects,
            SkPaint::FontMetrics*) = 0;
    virtual void generateMetrics(
            const SkTypefaceProxy& tf,
            const SkScalerContextRec& rec,
            const SkScalerContextEffects& effects,
            SkGlyph* glyph) = 0;
    virtual void generateImage(
            const SkTypefaceProxy& tf,
            const SkScalerContextRec& rec,
            const SkScalerContextEffects& effects,
            const SkGlyph& glyph)  = 0;
    virtual void generatePath(
            const SkTypefaceProxy& tf,
            const SkScalerContextRec& rec,
            const SkScalerContextEffects& effects,
            SkGlyphID glyph, SkPath* path) = 0;
};

class RemoteScalerContextNothing : public RemoteScalerContext {
public:
    void generateFontMetrics(
            const SkTypefaceProxy& tf,
            const SkScalerContextRec& rec,
            const SkScalerContextEffects& effects,
            SkPaint::FontMetrics*) override { }
    void generateMetrics(
            const SkTypefaceProxy& tf,
            const SkScalerContextRec& rec,
            const SkScalerContextEffects& effects,
            SkGlyph* glyph) override {}
    void generateImage(const SkTypefaceProxy& tf,
                       const SkScalerContextRec& rec,
                       const SkScalerContextEffects& effects,
                       const SkGlyph& glyph) override { }
    void generatePath(const SkTypefaceProxy& tf,
                      const SkScalerContextRec& rec,
                      const SkScalerContextEffects& effects,
                      SkGlyphID glyph, SkPath* path) override { }
};

std::unordered_map<SkFontID, sk_sp<SkTypeface>> gTypefaceMap;

// TODO: Move this to a common place. Original in SkPaint.
template <typename A>
static auto create_desc_for_scaler_context(
        const SkScalerContextRec& rec,
        const SkScalerContextEffects& effects,
        A alloc) -> decltype(alloc((size_t)0)) {

    SkBinaryWriteBuffer peBuffer, mfBuffer, raBuffer;
    int entryCount = 1;
    size_t descSize = sizeof(rec);

    if (effects.fPathEffect) {
        effects.fPathEffect->flatten(peBuffer);
        descSize += peBuffer.bytesWritten();
        entryCount += 1;
    }
    if (effects.fMaskFilter) {
        effects.fMaskFilter->flatten(mfBuffer);
        descSize += mfBuffer.bytesWritten();
        entryCount += 1;
    }
    if (effects.fRasterizer) {
        effects.fRasterizer->flatten(raBuffer);
        descSize += raBuffer.bytesWritten();
        entryCount += 1;
    }

    descSize += SkDescriptor::ComputeOverhead(entryCount);

    auto desc = alloc(descSize);

    desc->init();
    desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);

    auto add = [&desc](uint32_t tag, SkBinaryWriteBuffer* buffer) {
        buffer->writeToMemory(desc->addEntry(tag, buffer->bytesWritten(), nullptr));
    };

    if (effects.fPathEffect) {
        add(kPathEffect_SkDescriptorTag, &peBuffer);
    }
    if (effects.fMaskFilter) {
        add(kMaskFilter_SkDescriptorTag, &mfBuffer);
    }
    if (effects.fRasterizer) {
        add(kRasterizer_SkDescriptorTag, &raBuffer);
    }

    desc->computeChecksum();
    return desc;
}

std::unique_ptr<SkScalerContext> get_scaler_context(
        const SkTypefaceProxy& tf,
        const SkScalerContextRec& rec,
        const SkScalerContextEffects& effects) {

    auto i = gTypefaceMap.find(tf.fontID());
    if (i == gTypefaceMap.end()) {
        SK_ABORT("Id not found.");
    }

    auto realTf = (*i).second;

    auto alloc = [](size_t size) {
        return SkDescriptor::Alloc(size);
    };

    auto desc = create_desc_for_scaler_context(rec, effects, alloc);

    // TODO: track is_fixed properly
    auto scaler = realTf->createScalerContext(effects, desc.get(), false);

    return scaler;
}


class RemoteScalerContextSameThread : public RemoteScalerContext {
public:
    void generateFontMetrics(const SkTypefaceProxy& tf,
                             const SkScalerContextRec& rec,
                             const SkScalerContextEffects& effects,
                             SkPaint::FontMetrics* metrics) override {
        auto scalerContext = get_scaler_context(tf, rec, effects);

        scalerContext->getFontMetrics(metrics);
    }

    void generateMetrics(const SkTypefaceProxy& tf,
                         const SkScalerContextRec& rec,
                         const SkScalerContextEffects& effects,
                         SkGlyph* glyph) override {
        auto scalerContext = get_scaler_context(tf, rec, effects);
        scalerContext->getMetrics(glyph);
    }

    void generateImage(const SkTypefaceProxy& tf,
                       const SkScalerContextRec& rec,
                       const SkScalerContextEffects& effects,
                       const SkGlyph& glyph) override {
        auto scalerContext = get_scaler_context(tf, rec, effects);
        scalerContext->getImage(glyph);
    }
    void generatePath(const SkTypefaceProxy& tf,
                      const SkScalerContextRec& rec,
                      const SkScalerContextEffects& effects,
                      SkGlyphID glyph, SkPath* path) override {
        auto scalerContext = get_scaler_context(tf, rec, effects);
        scalerContext->getPath(glyph, path);
    }
};

SkScalerContextProxy::SkScalerContextProxy(
        sk_sp<SkTypeface> tf,
        const SkScalerContextEffects& effects,
        const SkDescriptor* desc)
    : SkScalerContext{std::move(tf), effects, desc}
    , fRemote{new RemoteScalerContextSameThread()} {}

void SkScalerContextProxy::generateMetrics(SkGlyph* glyph) {
    fRemote->generateMetrics(*this->typefaceProxy(), this->getRec(), this->getEffects(), glyph);
}

void SkScalerContextProxy::generateImage(const SkGlyph& glyph) {
    fRemote->generateImage(*this->typefaceProxy(), this->getRec(), this->getEffects(), glyph);
}

void SkScalerContextProxy::generatePath(SkGlyphID glyphID, SkPath* path) {
    fRemote->generatePath(*this->typefaceProxy(), this->getRec(), this->getEffects(), glyphID, path);
}

void SkScalerContextProxy::generateFontMetrics(SkPaint::FontMetrics* metrics) {
    fRemote->generateFontMetrics(*this->typefaceProxy(), this->getRec(), this->getEffects(), metrics);
}

SkTypefaceProxy* SkScalerContextProxy::typefaceProxy() {
    auto up = this->getTypeface();
    return (SkTypefaceProxy *)up;
}
