/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkColorData.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkClipStack.h"
#include "tools/ToolUtils.h"

#include <utility>

constexpr static SkIRect kDeviceRect = {0, 0, 600, 600};
constexpr static SkIRect kCoverRect = {50, 50, 550, 550};

namespace skiagm {

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Draws a clip that will exercise window rectangles if they are supported.
 */
class WindowRectanglesGM : public GM {
private:
    DrawResult coverClipStack(const SkClipStack&, SkCanvas*, SkString* errorMsg);

    SkISize getISize() override { return SkISize::Make(kDeviceRect.width(), kDeviceRect.height()); }
    SkString getName() const override { return SkString("windowrectangles"); }
    DrawResult onDraw(SkCanvas*, SkString* errorMsg) override;
};

DrawResult WindowRectanglesGM::coverClipStack(const SkClipStack& stack, SkCanvas* canvas,
                                                SkString* errorMsg) {
    SkPaint paint;
    paint.setColor(0xff00aa80);

    // Set up the canvas's clip to match our SkClipStack.
    SkClipStack::Iter iter(stack, SkClipStack::Iter::kBottom_IterStart);
    for (const SkClipStack::Element* element = iter.next(); element; element = iter.next()) {
        SkASSERT(!element->isReplaceOp());
        SkClipOp op = element->getOp();
        bool isAA = element->isAA();
        switch (element->getDeviceSpaceType()) {
            case SkClipStack::Element::DeviceSpaceType::kShader:
                canvas->clipShader(element->refShader(), op);
                break;
            case SkClipStack::Element::DeviceSpaceType::kPath:
                canvas->clipPath(element->getDeviceSpacePath(), op, isAA);
                break;
            case SkClipStack::Element::DeviceSpaceType::kRRect:
                canvas->clipRRect(element->getDeviceSpaceRRect(), op, isAA);
                break;
            case SkClipStack::Element::DeviceSpaceType::kRect:
                canvas->clipRect(element->getDeviceSpaceRect(), op, isAA);
                break;
            case SkClipStack::Element::DeviceSpaceType::kEmpty:
                canvas->clipRect({ 0, 0, 0, 0 }, SkClipOp::kIntersect, false);
                break;
        }
    }

    canvas->drawRect(SkRect::Make(kCoverRect), paint);
    return DrawResult::kOk;
}

DrawResult WindowRectanglesGM::onDraw(SkCanvas* canvas, SkString* errorMsg) {
    ToolUtils::draw_checkerboard(canvas, 0xffffffff, 0xffc6c3c6, 25);
    SkClipStack stack;
    stack.clipRect(SkRect::MakeXYWH(370.75, 80.25, 149, 100), SkMatrix::I(),
                   SkClipOp::kDifference, false);
    stack.clipRect(SkRect::MakeXYWH(80.25, 420.75, 150, 100), SkMatrix::I(),
                   SkClipOp::kDifference, true);
    stack.clipRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(200, 200, 200, 200), 60, 45),
                    SkMatrix::I(), SkClipOp::kDifference, true);

    SkRRect nine;
    nine.setNinePatch(SkRect::MakeXYWH(550 - 30.25 - 100, 370.75, 100, 150), 12, 35, 23, 20);
    stack.clipRRect(nine, SkMatrix::I(), SkClipOp::kDifference, true);

    SkRRect complx;
    SkVector complxRadii[4] = {{6, 4}, {8, 12}, {16, 24}, {48, 32}};
    complx.setRectRadii(SkRect::MakeXYWH(80.25, 80.75, 100, 149), complxRadii);
    stack.clipRRect(complx, SkMatrix::I(), SkClipOp::kDifference, false);

    return this->coverClipStack(stack, canvas, errorMsg);
}

DEF_GM( return new WindowRectanglesGM(); )
}  // namespace skiagm
