#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkTileMode.h"
#include "tools/Resources.h"

DEF_SIMPLE_GM(b168846821, canvas, 600, 600) {
    auto image = GetResourceAsImage("images/5.png");
    auto matrix = SkMatrix::MakeAll(.805607, 0, 2.75,
                                    0, .805607, 2.75,
                                    0,       0,    1);
    auto shader = image->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, &matrix,
            kLow_SkFilterQuality);
    SkPaint paint;
    paint.setShader(shader);

    //canvas->clipRect(SkRect::MakeLTRB(0, 0, 441, 441));

    SkRect rect = SkRect::MakeLTRB(1.375, 1.375, 439.625, 439.625);
    SkRRect rrect = SkRRect::MakeRectXY(rect, 11.0f, 11.0f);
    canvas->drawRRect(rrect, paint);
}
