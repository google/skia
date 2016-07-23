/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Some shared code used by both SkBigPicture and SkMiniPicture.
//   SkTextHunter   -- SkRecord visitor that returns true when the op draws text.
//   SkBitmapHunter -- SkRecord visitor that returns true when the op draws a bitmap or image.
//   SkPathCounter  -- SkRecord visitor that counts paths that draw slowly on the GPU.

#include "SkPathEffect.h"
#include "SkRecords.h"
#include "SkTLogic.h"

// N.B. This name is slightly historical: hunting season is now open for SkImages too.
struct SkBitmapHunter {
    // Some ops have a paint, some have an optional paint.  Either way, get back a pointer.
    static const SkPaint* AsPtr(const SkPaint& p) { return &p; }
    static const SkPaint* AsPtr(const SkRecords::Optional<SkPaint>& p) { return p; }

    // Main entry for visitor:
    // If the op is a DrawPicture, recurse.
    // If the op has a bitmap or image directly, return true.
    // If the op has a paint and the paint has a bitmap, return true.
    // Otherwise, return false.
    bool operator()(const SkRecords::DrawPicture& op) { return op.picture->willPlayBackBitmaps(); }
    bool operator()(const SkRecords::DrawDrawable&) { /*TODO*/ return false; }

    template <typename T>
    bool operator()(const T& op) { return CheckBitmap(op); }

    // If the op is tagged as having an image, return true.
    template <typename T>
    static SK_WHEN(T::kTags & SkRecords::kHasImage_Tag, bool) CheckBitmap(const T&) {
        return true;
    }

    // If not, look for one in its paint (if it has a paint).
    template <typename T>
    static SK_WHEN(!(T::kTags & SkRecords::kHasImage_Tag), bool) CheckBitmap(const T& op) {
        return CheckPaint(op);
    }

    // Most draws-type ops have paints.
    template <typename T>
    static SK_WHEN(T::kTags & SkRecords::kDraw_Tag, bool) CheckPaint(const T& op) {
        return PaintHasBitmap(AsPtr(op.paint));
    }

    // SaveLayers also have a paint to check.
    static bool CheckPaint(const SkRecords::SaveLayer& op) {
        return PaintHasBitmap(AsPtr(op.paint));
    }

    // Shouldn't be any non-Draw non-SaveLayer ops with paints.
    template <typename T>
    static SK_WHEN(!(T::kTags & SkRecords::kDraw_Tag), bool) CheckPaint(const T&) {
        return false;
    }

private:
    static bool PaintHasBitmap(const SkPaint* paint) {
        if (paint) {
            const SkShader* shader = paint->getShader();
            if (shader && shader->isABitmap()) {
                return true;
            }
        }
        return false;
    }
};

// TODO: might be nicer to have operator() return an int (the number of slow paths) ?
struct SkPathCounter {
    // Some ops have a paint, some have an optional paint.  Either way, get back a pointer.
    static const SkPaint* AsPtr(const SkPaint& p) { return &p; }
    static const SkPaint* AsPtr(const SkRecords::Optional<SkPaint>& p) { return p; }

    SkPathCounter() : fNumSlowPathsAndDashEffects(0) {}

    // Recurse into nested pictures.
    void operator()(const SkRecords::DrawPicture& op) {
        fNumSlowPathsAndDashEffects += op.picture->numSlowPaths();
    }
    void operator()(const SkRecords::DrawDrawable&) { /* TODO */ }

    void checkPaint(const SkPaint* paint) {
        if (paint && paint->getPathEffect()) {
            // Initially assume it's slow.
            fNumSlowPathsAndDashEffects++;
        }
    }

    void operator()(const SkRecords::DrawPoints& op) {
        this->checkPaint(&op.paint);
        const SkPathEffect* effect = op.paint.getPathEffect();
        if (effect) {
            SkPathEffect::DashInfo info;
            SkPathEffect::DashType dashType = effect->asADash(&info);
            if (2 == op.count && SkPaint::kRound_Cap != op.paint.getStrokeCap() &&
                SkPathEffect::kDash_DashType == dashType && 2 == info.fCount) {
                fNumSlowPathsAndDashEffects--;
            }
        }
    }

    void operator()(const SkRecords::DrawPath& op) {
        this->checkPaint(&op.paint);
        if (op.paint.isAntiAlias() && !op.path.isConvex()) {
            SkPaint::Style paintStyle = op.paint.getStyle();
            const SkRect& pathBounds = op.path.getBounds();
            if (SkPaint::kStroke_Style == paintStyle &&
                0 == op.paint.getStrokeWidth()) {
                // AA hairline concave path is not slow.
            } else if (SkPaint::kFill_Style == paintStyle && pathBounds.width() < 64.f &&
                       pathBounds.height() < 64.f && !op.path.isVolatile()) {
                // AADF eligible concave path is not slow.
            } else {
                fNumSlowPathsAndDashEffects++;
            }
        }
    }

    void operator()(const SkRecords::ClipPath& op) {
        // TODO: does the SkRegion op matter?
        if (op.opAA.aa && !op.path.isConvex()) {
            fNumSlowPathsAndDashEffects++;
        }
    }

    void operator()(const SkRecords::SaveLayer& op) {
        this->checkPaint(AsPtr(op.paint));
    }

    template <typename T>
    SK_WHEN(T::kTags & SkRecords::kDraw_Tag, void) operator()(const T& op) {
        this->checkPaint(AsPtr(op.paint));
    }

    template <typename T>
    SK_WHEN(!(T::kTags & SkRecords::kDraw_Tag), void) operator()(const T& op) { /* do nothing */ }

    int fNumSlowPathsAndDashEffects;
};
