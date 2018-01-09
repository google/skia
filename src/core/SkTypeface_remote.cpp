/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypeface_remote.h"
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


SkScalerContext* get_scaler_context(const SkTypefaceProxy& tf, const SkScalerContextRec& rec,
                                    const SkScalerContextEffects& effects) {
    //auto rtf = gTypefaceMap.find(tf.fontID());

    return nullptr;
}


class RemoteScalerContextSameThread : public RemoteScalerContext {
public:
    void generateFontMetrics(const SkTypefaceProxy& tf,
                             const SkScalerContextRec& rec,
                             const SkScalerContextEffects& effects,
                             SkPaint::FontMetrics* metrics) override {
        SkScalerContext* scalerContext = get_scaler_context(tf, rec, effects);

        scalerContext->getFontMetrics(metrics);

    }
    void generateImage(const SkTypefaceProxy& tf,
                       const SkScalerContextRec& rec,
                       const SkScalerContextEffects& effects,
                       const SkGlyph& glyph) override {
        SkScalerContext* scalerContext = get_scaler_context(tf, rec, effects);
        scalerContext->getImage(glyph);
    }
    void generatePath(const SkTypefaceProxy& tf,
                      const SkScalerContextRec& rec,
                      const SkScalerContextEffects& effects,
                      SkGlyphID glyph, SkPath* path) override {
        SkScalerContext* scalerContext = get_scaler_context(tf, rec, effects);
        scalerContext->getPath(glyph, path);
    }
};

SkScalerContextProxy::SkScalerContextProxy(
        sk_sp<SkTypeface> tf,
        const SkScalerContextEffects& effects,
        const SkDescriptor* desc)
    : SkScalerContext{std::move(tf), effects, desc}
    , fRemote{new RemoteScalerContextNothing()} {}

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
