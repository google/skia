/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// The purpose of this Bazel-only GM is to experiment with a task driver that uploads PNGs produced
// by GMs executed via Bazel. By creating a GM specific for these experiments, we avoid uploading
// spurious digests with potentially incorrect keys that would be grouped with existing digests.
//
// Based on //gm/bigtext.cpp.
//
// TODO(lovisolo): Delete once we migrate other GMs to Bazel.

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

class HelloBazelWorldGM : public skiagm::GM {
public:
    HelloBazelWorldGM() {}

protected:
    SkString getName() const override { return SkString("HelloBazelWorld"); }

    SkISize getISize() override { return SkISize::Make(500, 500); }

    bool isBazelOnly() const override { return true; }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        SkFont font(ToolUtils::create_portable_typeface(), 50);

        const char* text = "Hello, Bazel world!";
        size_t text_length = strlen(text);

        SkRect r;
        (void)font.measureText(text, text_length, SkTextEncoding::kUTF8, &r);
        SkPoint pos = {this->width() / 2 - r.centerX(), this->height() / 2 - r.centerY()};

        paint.setColor(SK_ColorRED);
        canvas->drawSimpleText(
                text, text_length, SkTextEncoding::kUTF8, pos.fX, pos.fY, font, paint);
    }
};

DEF_GM(return new HelloBazelWorldGM;)
