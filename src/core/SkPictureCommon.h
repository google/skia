/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureCommon_DEFINED
#define SkPictureCommon_DEFINED

// Some shared code used by both SkBigPicture and SkMiniPicture.
//   SkTextHunter   -- SkRecord visitor that returns true when the op draws text.
//   SkPathCounter  -- SkRecord visitor that counts paths that draw slowly on the GPU.

#include "include/core/SkPathEffect.h"
#include "include/core/SkShader.h"
#include "include/private/SkTLogic.h"
#include "src/core/SkRecords.h"

// TODO: might be nicer to have operator() return an int (the number of slow paths) ?
struct SkPathCounter {
    // Some ops have a paint, some have an optional paint.  Either way, get back a pointer.
    static const SkPaint* AsPtr(const SkPaint& p) { return &p; }
    static const SkPaint* AsPtr(const SkRecords::Optional<SkPaint>& p) { return p; }

    SkPathCounter() : fNumSlowPathsAndDashEffects(0) {}

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
        if (op.opAA.aa() && !op.path.isConvex()) {
            fNumSlowPathsAndDashEffects++;
        }
    }

    void operator()(const SkRecords::SaveLayer& op) {
        this->checkPaint(AsPtr(op.paint));
    }

    template <typename T>
    SK_WHEN(T::kTags & SkRecords::kHasPaint_Tag, void) operator()(const T& op) {
        this->checkPaint(AsPtr(op.paint));
    }

    template <typename T>
    SK_WHEN(!(T::kTags & SkRecords::kHasPaint_Tag), void)
      operator()(const T& op) { /* do nothing */ }

    int fNumSlowPathsAndDashEffects;
};

sk_sp<SkImage> ImageDeserializer_SkDeserialImageProc(const void*, size_t, void* imagedeserializer);

bool SkPicture_StreamIsSKP(SkStream*, SkPictInfo*);

#endif  // SkPictureCommon_DEFINED
