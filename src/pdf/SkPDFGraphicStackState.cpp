// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/pdf/SkPDFGraphicStackState.h"

#include "include/core/SkStream.h"
#include "include/pathops/SkPathOps.h"
#include "src/pdf/SkPDFUtils.h"
#include "src/utils/SkClipStackUtils.h"

static void emit_pdf_color(SkColor4f color, SkWStream* result) {
    SkASSERT(color.fA == 1);  // We handle alpha elsewhere.
    SkPDFUtils::AppendColorComponentF(color.fR, result);
    result->writeText(" ");
    SkPDFUtils::AppendColorComponentF(color.fG, result);
    result->writeText(" ");
    SkPDFUtils::AppendColorComponentF(color.fB, result);
    result->writeText(" ");
}

static SkRect rect_intersect(SkRect u, SkRect v) {
    if (u.isEmpty() || v.isEmpty()) { return {0, 0, 0, 0}; }
    return u.intersect(v) ? u : SkRect{0, 0, 0, 0};
}

// Test to see if the clipstack is a simple rect, If so, we can avoid all PathOps code
// and speed thing up.
static bool is_rect(const SkClipStack& clipStack, const SkRect& bounds, SkRect* dst) {
    SkRect currentClip = bounds;
    SkClipStack::Iter iter(clipStack, SkClipStack::Iter::kBottom_IterStart);
    while (const SkClipStack::Element* element = iter.next()) {
        SkRect elementRect{0, 0, 0, 0};
        switch (element->getDeviceSpaceType()) {
            case SkClipStack::Element::DeviceSpaceType::kEmpty:
                break;
            case SkClipStack::Element::DeviceSpaceType::kRect:
                elementRect = element->getDeviceSpaceRect();
                break;
            default:
                return false;
        }
        if (element->isReplaceOp()) {
            currentClip = rect_intersect(bounds, elementRect);
        } else if (element->getOp() == SkClipOp::kIntersect) {
            currentClip = rect_intersect(currentClip, elementRect);
        } else {
            return false;
        }
    }
    *dst = currentClip;
    return true;
}

// TODO: When there's no expanding clip ops, this function may not be necessary anymore.
static bool is_complex_clip(const SkClipStack& stack) {
    SkClipStack::Iter iter(stack, SkClipStack::Iter::kBottom_IterStart);
    while (const SkClipStack::Element* element = iter.next()) {
        if (element->isReplaceOp() ||
            (element->getOp() != SkClipOp::kDifference &&
             element->getOp() != SkClipOp::kIntersect)) {
            return true;
        }
    }
    return false;
}

template <typename F>
static void apply_clip(const SkClipStack& stack, const SkRect& outerBounds, F fn) {
    // assumes clipstack is not complex.
    constexpr SkRect kHuge{-30000, -30000, 30000, 30000};
    SkClipStack::Iter iter(stack, SkClipStack::Iter::kBottom_IterStart);
    SkRect bounds = outerBounds;
    while (const SkClipStack::Element* element = iter.next()) {
        SkPath operand;
        element->asDeviceSpacePath(&operand);
        SkPathOp op;
        switch (element->getOp()) {
            case SkClipOp::kDifference: op = kDifference_SkPathOp; break;
            case SkClipOp::kIntersect:  op = kIntersect_SkPathOp;  break;
            default: SkASSERT(false); return;
        }
        if (op == kDifference_SkPathOp  ||
            operand.isInverseFillType() ||
            !kHuge.contains(operand.getBounds()))
        {
            Op(SkPath::Rect(bounds), operand, op, &operand);
        }
        SkASSERT(!operand.isInverseFillType());
        fn(operand);
        if (!bounds.intersect(operand.getBounds())) {
            return; // return early;
        }
    }
}

static void append_clip_path(const SkPath& clipPath, SkWStream* wStream) {
    SkPDFUtils::EmitPath(clipPath, SkPaint::kFill_Style, wStream);
    SkPathFillType clipFill = clipPath.getFillType();
    NOT_IMPLEMENTED(clipFill == SkPathFillType::kInverseEvenOdd, false);
    NOT_IMPLEMENTED(clipFill == SkPathFillType::kInverseWinding, false);
    if (clipFill == SkPathFillType::kEvenOdd) {
        wStream->writeText("W* n\n");
    } else {
        wStream->writeText("W n\n");
    }
}

static void append_clip(const SkClipStack& clipStack,
                        const SkIRect& bounds,
                        SkWStream* wStream) {
    // The bounds are slightly outset to ensure this is correct in the
    // face of floating-point accuracy and possible SkRegion bitmap
    // approximations.
    SkRect outsetBounds = SkRect::Make(bounds.makeOutset(1, 1));

    SkRect clipStackRect;
    if (is_rect(clipStack, outsetBounds, &clipStackRect)) {
        SkPDFUtils::AppendRectangle(clipStackRect, wStream);
        wStream->writeText("W* n\n");
        return;
    }

    if (is_complex_clip(clipStack)) {
        SkPath clipPath;
        SkClipStack_AsPath(clipStack, &clipPath);
        if (Op(clipPath, SkPath::Rect(outsetBounds), kIntersect_SkPathOp, &clipPath)) {
            append_clip_path(clipPath, wStream);
        }
        // If Op() fails (pathological case; e.g. input values are
        // extremely large or NaN), emit no clip at all.
    } else {
        apply_clip(clipStack, outsetBounds, [wStream](const SkPath& path) {
            append_clip_path(path, wStream);
        });
    }
}

////////////////////////////////////////////////////////////////////////////////

void SkPDFGraphicStackState::updateClip(const SkClipStack* clipStack, const SkIRect& bounds) {
    uint32_t clipStackGenID = clipStack ? clipStack->getTopmostGenID()
                                        : SkClipStack::kWideOpenGenID;
    if (clipStackGenID == currentEntry()->fClipStackGenID) {
        return;
    }
    while (fStackDepth > 0) {
        this->pop();
        if (clipStackGenID == currentEntry()->fClipStackGenID) {
            return;
        }
    }
    SkASSERT(currentEntry()->fClipStackGenID == SkClipStack::kWideOpenGenID);
    if (clipStackGenID != SkClipStack::kWideOpenGenID) {
        SkASSERT(clipStack);
        this->push();

        currentEntry()->fClipStackGenID = clipStackGenID;
        append_clip(*clipStack, bounds, fContentStream);
    }
}


void SkPDFGraphicStackState::updateMatrix(const SkMatrix& matrix) {
    if (matrix == currentEntry()->fMatrix) {
        return;
    }

    if (currentEntry()->fMatrix.getType() != SkMatrix::kIdentity_Mask) {
        SkASSERT(fStackDepth > 0);
        SkASSERT(fEntries[fStackDepth].fClipStackGenID ==
                 fEntries[fStackDepth -1].fClipStackGenID);
        this->pop();

        SkASSERT(currentEntry()->fMatrix.getType() == SkMatrix::kIdentity_Mask);
    }
    if (matrix.getType() == SkMatrix::kIdentity_Mask) {
        return;
    }

    this->push();
    SkPDFUtils::AppendTransform(matrix, fContentStream);
    currentEntry()->fMatrix = matrix;
}

void SkPDFGraphicStackState::updateDrawingState(const SkPDFGraphicStackState::Entry& state) {
    // PDF treats a shader as a color, so we only set one or the other.
    if (state.fShaderIndex >= 0) {
        if (state.fShaderIndex != currentEntry()->fShaderIndex) {
            SkPDFUtils::ApplyPattern(state.fShaderIndex, fContentStream);
            currentEntry()->fShaderIndex = state.fShaderIndex;
        }
    } else {
        if (state.fColor != currentEntry()->fColor ||
                currentEntry()->fShaderIndex >= 0) {
            emit_pdf_color(state.fColor, fContentStream);
            fContentStream->writeText("RG ");
            emit_pdf_color(state.fColor, fContentStream);
            fContentStream->writeText("rg\n");
            currentEntry()->fColor = state.fColor;
            currentEntry()->fShaderIndex = -1;
        }
    }

    if (state.fGraphicStateIndex != currentEntry()->fGraphicStateIndex) {
        SkPDFUtils::ApplyGraphicState(state.fGraphicStateIndex, fContentStream);
        currentEntry()->fGraphicStateIndex = state.fGraphicStateIndex;
    }

    if (state.fTextScaleX) {
        if (state.fTextScaleX != currentEntry()->fTextScaleX) {
            SkScalar pdfScale = state.fTextScaleX * 100;
            SkPDFUtils::AppendScalar(pdfScale, fContentStream);
            fContentStream->writeText(" Tz\n");
            currentEntry()->fTextScaleX = state.fTextScaleX;
        }
    }
}

void SkPDFGraphicStackState::push() {
    SkASSERT(fStackDepth < kMaxStackDepth);
    fContentStream->writeText("q\n");
    ++fStackDepth;
    fEntries[fStackDepth] = fEntries[fStackDepth - 1];
}

void SkPDFGraphicStackState::pop() {
    SkASSERT(fStackDepth > 0);
    fContentStream->writeText("Q\n");
    fEntries[fStackDepth] = SkPDFGraphicStackState::Entry();
    --fStackDepth;
}

void SkPDFGraphicStackState::drainStack() {
    if (fContentStream) {
        while (fStackDepth) {
            this->pop();
        }
    }
    SkASSERT(fStackDepth == 0);
}
