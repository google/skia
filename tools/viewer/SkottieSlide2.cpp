/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieSlide.h"

#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "Skottie.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkStream.h"

const int CELL_WIDTH  = 240;
const int CELL_HEIGHT = 160;
const int COL_COUNT   = 4;
const int SPACER_X    = 12;
const int SPACER_Y    = 24;
const int MARGIN      = 8;

SkottieSlide2::Rec::Rec(std::unique_ptr<skottie::Animation> anim) : fAnimation(std::move(anim))
{}

SkottieSlide2::Rec::Rec(Rec&& o)
    : fAnimation(std::move(o.fAnimation))
    , fTimeBase(o.fTimeBase)
    , fName(o.fName)
    , fShowAnimationInval(o.fShowAnimationInval)
{}

SkottieSlide2::SkottieSlide2(const SkString& path)
    : fPath(path)
{
    fName.set("skottie-dir");
}

void SkottieSlide2::load(SkScalar, SkScalar) {
    SkString name;
    SkOSFile::Iter iter(fPath.c_str(), "json");
    while (iter.next(&name)) {
        SkString path = SkOSPath::Join(fPath.c_str(), name.c_str());
        if (auto anim  = skottie::Animation::MakeFromFile(path.c_str())) {
            fAnims.push_back(Rec(std::move(anim))).fName = name;
        }
    }
}

void SkottieSlide2::unload() {
    fAnims.reset();
}

SkISize SkottieSlide2::getDimensions() const {
    const int rows = (fAnims.count() + COL_COUNT - 1) / COL_COUNT;
    return {
        MARGIN + (COL_COUNT - 1) * SPACER_X + COL_COUNT * CELL_WIDTH + MARGIN,
        MARGIN + (rows - 1) * SPACER_Y + rows * CELL_HEIGHT + MARGIN,
    };
}

void SkottieSlide2::draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(12);
    paint.setAntiAlias(true);
    paint.setTextAlign(SkPaint::kCenter_Align);

    const SkRect dst = SkRect::MakeIWH(CELL_WIDTH, CELL_HEIGHT);
    int x = 0, y = 0;

    canvas->translate(MARGIN, MARGIN);
    for (const auto& rec : fAnims) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(x * (CELL_WIDTH + SPACER_X), y * (CELL_HEIGHT + SPACER_Y));
        canvas->drawText(rec.fName.c_str(), rec.fName.size(),
                         dst.centerX(), dst.bottom() + paint.getTextSize(), paint);
        rec.fAnimation->render(canvas, &dst);
        if (++x == COL_COUNT) {
            x = 0;
            y += 1;
        }
    }
}

bool SkottieSlide2::animate(const SkAnimTimer& timer) {
    for (auto& rec : fAnims) {
        if (rec.fTimeBase == 0) {
            // Reset the animation time.
            rec.fTimeBase = timer.msec();
        }
        rec.fAnimation->animationTick(timer.msec() - rec.fTimeBase);
    }
    return true;
}

bool SkottieSlide2::onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState state,
                           uint32_t modifiers) {
    if (fTrackingCell < 0 && state == sk_app::Window::kDown_InputState) {
        fTrackingCell = this->findCell(x, y);
    }
    if (fTrackingCell >= 0 && state == sk_app::Window::kUp_InputState) {
        int index = this->findCell(x, y);
        if (fTrackingCell == index) {
            fAnims[index].fShowAnimationInval = !fAnims[index].fShowAnimationInval;
            fAnims[index].fAnimation->setShowInval(fAnims[index].fShowAnimationInval);
        }
        fTrackingCell = -1;
    }
    return fTrackingCell >= 0;
}

int SkottieSlide2::findCell(float x, float y) const {
    x -= MARGIN;
    y -= MARGIN;
    int index = -1;
    if (x >= 0 && y >= 0) {
        int ix = (int)x;
        int iy = (int)y;
        int col = ix / (CELL_WIDTH + SPACER_X);
        int row = iy / (CELL_HEIGHT + SPACER_Y);
        index = row * COL_COUNT + col;
        if (index >= fAnims.count()) {
            index = -1;
        }
    }
    return index;
}
