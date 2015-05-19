/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Some shared code used by both SkBigPicture and SkMiniPicture.
//   SkTextHunter   -- SkRecord visitor that returns true when the op draws text.
//   SkBitmapHunter -- SkRecord visitor that returns true when the op draws a bitmap.
//   SkPathCounter  -- SkRecord visitor that counts paths that draw slowly on the GPU.

#include "SkPathEffect.h"
#include "SkRecords.h"
#include "SkTLogic.h"

struct SkTextHunter {
    // Most ops never have text.  Some always do.  Subpictures know themeselves.
    template <typename T> bool operator()(const T&) { return false; }
    bool operator()(const SkRecords::DrawPosText&)    { return true; }
    bool operator()(const SkRecords::DrawPosTextH&)   { return true; }
    bool operator()(const SkRecords::DrawText&)       { return true; }
    bool operator()(const SkRecords::DrawTextBlob&)   { return true; }
    bool operator()(const SkRecords::DrawTextOnPath&) { return true; }
    bool operator()(const SkRecords::DrawPicture& op) { return op.picture->hasText(); }
};


struct SkBitmapHunter {
    // Helpers.  These create HasMember_bitmap and HasMember_paint.
    SK_CREATE_MEMBER_DETECTOR(bitmap);
    SK_CREATE_MEMBER_DETECTOR(paint);

    // Some ops have a paint, some have an optional paint.  Either way, get back a pointer.
    static const SkPaint* AsPtr(const SkPaint& p) { return &p; }
    static const SkPaint* AsPtr(const SkRecords::Optional<SkPaint>& p) { return p; }

    // Main entry for visitor:
    // If the op is a DrawPicture, recurse.
    // If the op has a bitmap directly, return true.
    // If the op has a paint and the paint has a bitmap, return true.
    // Otherwise, return false.
    bool operator()(const SkRecords::DrawPicture& op) { return op.picture->willPlayBackBitmaps(); }

    template <typename T>
    bool operator()(const T& r) { return CheckBitmap(r); }

    // If the op has a bitmap, of course we're going to play back bitmaps.
    template <typename T>
    static SK_WHEN(HasMember_bitmap<T>, bool) CheckBitmap(const T&) { return true; }

    // If not, look for one in its paint (if it has a paint).
    template <typename T>
    static SK_WHEN(!HasMember_bitmap<T>, bool) CheckBitmap(const T& r) { return CheckPaint(r); }

    // If we have a paint, dig down into the effects looking for a bitmap.
    template <typename T>
    static SK_WHEN(HasMember_paint<T>, bool) CheckPaint(const T& r) {
        const SkPaint* paint = AsPtr(r.paint);
        if (paint) {
            const SkShader* shader = paint->getShader();
            if (shader &&
                shader->asABitmap(nullptr, nullptr, nullptr) == SkShader::kDefault_BitmapType) {
                return true;
            }
        }
        return false;
    }

    // If we don't have a paint, that non-paint has no bitmap.
    template <typename T>
    static SK_WHEN(!HasMember_paint<T>, bool) CheckPaint(const T&) { return false; }
};

// TODO: might be nicer to have operator() return an int (the number of slow paths) ?
struct SkPathCounter {
    SK_CREATE_MEMBER_DETECTOR(paint);

    // Some ops have a paint, some have an optional paint.  Either way, get back a pointer.
    static const SkPaint* AsPtr(const SkPaint& p) { return &p; }
    static const SkPaint* AsPtr(const SkRecords::Optional<SkPaint>& p) { return p; }

    SkPathCounter() : fNumSlowPathsAndDashEffects(0) {}

    // Recurse into nested pictures.
    void operator()(const SkRecords::DrawPicture& op) {
        fNumSlowPathsAndDashEffects += op.picture->numSlowPaths();
    }

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

    template <typename T>
    SK_WHEN(HasMember_paint<T>, void) operator()(const T& op) {
        this->checkPaint(AsPtr(op.paint));
    }

    template <typename T>
    SK_WHEN(!HasMember_paint<T>, void) operator()(const T& op) { /* do nothing */ }

    int fNumSlowPathsAndDashEffects;
};
