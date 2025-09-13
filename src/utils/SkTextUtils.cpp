/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkTextUtils.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTextBlob.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkFontPriv.h"

using namespace skia_private;

class SkPaint;

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
    AutoTArray<SkPoint> pos(ag.count());
    font.getPos(ag, pos, {x, y});

    struct Rec {
        SkPath* fDst;
        const SkPoint* fPos;
    } rec = { path, pos.get() };

    path->reset();
    font.getPaths(ag, [](const SkPath* src, const SkMatrix& mx, void* ctx) {
        Rec* rec = (Rec*)ctx;
        if (src) {
            SkMatrix m(mx);
            m.postTranslate(rec->fPos->fX, rec->fPos->fY);
            rec->fDst->addPath(*src, m);
        }
        rec->fPos += 1;
    }, &rec);
}

