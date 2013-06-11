/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkFontHost.h"
#include "SkPaint.h"
#include "SkString.h"
#include "SkTemplates.h"

#include "gUniqueGlyphIDs.h"

class FontCacheBench : public SkBenchmark {
    enum { N = SkBENCHLOOP(40) };
public:
    FontCacheBench(void* param) : INHERITED(param) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "fontcache";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        this->setupPaint(&paint);
        paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

        for (int i = 0; i < N; ++i) {
            const uint16_t* array = gUniqueGlyphIDs;
            while (*array != 0xFFFF) {
                const uint16_t* end = array + 1;
                while (*end != 0xFFFF) {
                    end += 1;
                }
                paint.measureText(array, end - array);
                array = end + 1;    // skip the sentinel
            }
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new FontCacheBench(p); )
