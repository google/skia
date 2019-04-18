/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontPriv.h"
#include "SkPath.h"
#include "SkTextUtils.h"
#include "SkTextBlob.h"

void SkTextUtils::Draw(SkCanvas* canvas, const void* text, size_t size, SkTextEncoding encoding,
                       SkScalar x, SkScalar y, const SkFont& font, const SkPaint& paint,
                       Align align) {
    if (align != kLeft_Align) {
        SkScalar width = font.measureText(text, size, encoding);
        if (align == kCenter_Align) {
            width *= 0.5f;
        }
        x -= width;
    }

    canvas->drawTextBlob(SkTextBlob::MakeFromText(text, size, font, encoding), x, y, paint);
}

void SkTextUtils::GetPath(const void* text, size_t length, SkTextEncoding encoding,
                          SkScalar x, SkScalar y, const SkFont& font, SkPath* path) {
    SkAutoToGlyphs ag(font, text, length, encoding);
    SkAutoTArray<SkPoint> pos(ag.count());
    font.getPos(ag.glyphs(), ag.count(), pos.get(), {x, y});

    struct Rec {
        SkPath* fDst;
        const SkPoint* fPos;
    } rec = { path, pos.get() };

    path->reset();
    font.getPaths(ag.glyphs(), ag.count(), [](const SkPath* src, const SkMatrix& mx, void* ctx) {
        Rec* rec = (Rec*)ctx;
        if (src) {
            SkMatrix m(mx);
            m.postTranslate(rec->fPos->fX, rec->fPos->fY);
            rec->fDst->addPath(*src, m);
        }
        rec->fPos += 1;
    }, &rec);
}

