/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"

class CrBug531782512GM : public skiagm::GM {
public:
    CrBug531782512GM() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:
    SkString getName() const override { return SkString("crbug_531782512"); }

    SkISize getISize() override { return SkISize::Make(1050, 100); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
#if defined(SK_GRAPHITE)
        if (!canvas->recorder()) {
            *errorMsg = "Graphite only";
            return DrawResult::kSkip;
        }

        canvas->clear(SK_ColorBLACK);

        // Create a non-rectangular path that is wider than the draw atlas slot width limit (1022
        // pixels) to force it to use the proxy cache path.
        SkPathBuilder builder;
        builder.moveTo(10, 10);
        builder.lineTo(1040, 10);
        builder.lineTo(525, 90);
        builder.close();
        SkPath path = builder.detach();

        canvas->save();
        canvas->clipPath(path, true);

        // Draw a horizontal hairline along the top edge of the clip bounds, if the pixel insetting
        // and translation into the clip atlas is incorrect, the line will not draw.
        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(0); // hairline
        canvas->drawLine(10, 10.5f, 1040, 10.5f, paint);

        canvas->restore();
        return DrawResult::kOk;
#else
        *errorMsg = "Graphite only";
        return DrawResult::kSkip;
#endif
    }
};

DEF_GM(return new CrBug531782512GM;)
