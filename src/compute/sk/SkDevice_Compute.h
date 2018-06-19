/*
 * Copyright 2016 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef SkDevice_Compute_DEFINED
#define SkDevice_Compute_DEFINED

//
// for now make sure it's defined
//

#if !defined(SK_SUPPORT_GPU_COMPUTE)
#define SK_SUPPORT_GPU_COMPUTE 1
#endif

//
//
//

#if SK_SUPPORT_GPU_COMPUTE

// TODO Check whether we can use SkDevice_ComputeLayerGroup at compile time
// by checking whether there is only one top device.
#define SK_USE_COMPUTE_LAYER_GROUP

//
// C
//

#ifdef __cplusplus
extern "C" {
#endif

#include <context.h>

#ifdef __cplusplus
}
#endif

#include "../compute/skc/skc.h"

//
// C++
//

#include "SkDevice.h"
#include "SkClipStackDevice.h"
#include "SkContext_Compute.h"
#include "SkTArray.h"

//
//
//

#ifdef SK_USE_COMPUTE_LAYER_GROUP
class SkDevice_ComputeLayerGroup;
#endif

class SkDevice_Compute : public SkClipStackDevice {
public:
    SkDevice_Compute(sk_sp<SkContext_Compute>, int w, int h);
    ~SkDevice_Compute() override;

    void drawPaint(const SkPaint& paint) override;
    void drawPoints(SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&) override;
    void drawRect(const SkRect&, const SkPaint&) override;
    void drawOval(const SkRect&, const SkPaint&) override;
    void drawRRect(const SkRRect&, const SkPaint&) override;
    void drawPath(const SkPath&, const SkPaint&, const SkMatrix*, bool) override;
    void drawText(const void*, size_t, SkScalar, SkScalar, const SkPaint&) override;
    void drawPosText(const void*, size_t, const SkScalar[], int, const SkPoint&,
                     const SkPaint&) override;

    void onRestore() override {
        this->SkClipStackDevice::onRestore();
        fClipWeakref = SKC_WEAKREF_INVALID;
    }
    void onClipRect(const SkRect& rect, SkClipOp op, bool aa) override {
        this->SkClipStackDevice::onClipRect(rect, op, aa);
        fClipWeakref = SKC_WEAKREF_INVALID;
    }
    void onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) override {
        this->SkClipStackDevice::onClipRRect(rrect, op, aa);
        fClipWeakref = SKC_WEAKREF_INVALID;
    }
    void onClipPath(const SkPath& path, SkClipOp op, bool aa) override {
        this->SkClipStackDevice::onClipPath(path, op, aa);
        fClipWeakref = SKC_WEAKREF_INVALID;
    }
    void onClipRegion(const SkRegion& deviceRgn, SkClipOp op) override {
        this->SkClipStackDevice::onClipRegion(deviceRgn, op);
        fClipWeakref = SKC_WEAKREF_INVALID;
    }
    void onSetDeviceClipRestriction(SkIRect* clipRestriction) override {
        this->SkClipStackDevice::onSetDeviceClipRestriction(clipRestriction);
        fClipWeakref = SKC_WEAKREF_INVALID;
    }

    ClipType onGetClipType() const override {
        // TODO Support non-rect clip
        return kRect_ClipType;
    }

    void drawBitmap(const SkBitmap&, const SkMatrix&, const SkPaint&) override {}
    void drawSprite(const SkBitmap&, int, int, const SkPaint&) override {}
    void drawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint&,
                        SkCanvas::SrcRectConstraint) override {}
    void drawDevice(SkBaseDevice*, int, int, const SkPaint&) override;
    void drawVertices(const SkVertices*, SkBlendMode, const SkPaint&) override {}
    void flush() override;

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    void onCtmChanged() override;

    friend class SkDevice_ComputeLayerGroup;

private:
    void styling_group_init();

    void path_add(const SkPaint&, const SkPath&, const SkMatrix* prePathMatrix = nullptr);
    void circles_add(const SkPaint&, const SkPoint points[], int32_t count, SkScalar radius);
    void squares_add(const SkPaint&, const SkPoint points[], int32_t count, SkScalar radius);
    void line_stroked_butt(SkPoint xy0, SkPoint xy1, SkScalar radius);
    void lines_stroked_add(const SkPaint&, const SkPoint points[], int32_t count, SkScalar radius);
    void path_rasterize_and_place(const SkPaint&, const skc_path_t path,
                                  const SkMatrix* prePathMatrix = nullptr);

    sk_sp<SkContext_Compute> fCompute;

    skc_composition_t    fComposition;
    skc_styling_t        fStyling;

    skc_path_builder_t   fPB;
    skc_raster_builder_t fRB;

    skc_group_id         fGroupID;
    skc_group_id         fGroupLayerID;

    // When SK_USE_COMPUTE_LAYER_GROUP is set, fTopCTM is the global CTM for the top device.
    // When SK_USE_COMPUTE_LAYER_GROUP is not set, fTopCTM is equal to this->ctm().
    SkMatrix                fTopCTM;
    skc_transform_weakref_t fTransformWeakref;

    skc_raster_clip_weakref_t fClipWeakref;

#ifdef SK_USE_COMPUTE_LAYER_GROUP
    SkTArray<skc_group_id> fParents;

    SkDevice_ComputeLayerGroup* createLayerGroup(const CreateInfo&, const SkPaint*);
#endif
};

#ifdef SK_USE_COMPUTE_LAYER_GROUP

// A group of skc layers that correspond to a saveLayer in the top level (root) SkDevice_Compute.
class SkDevice_ComputeLayerGroup : public SkBaseDevice {
public:
    SkDevice_ComputeLayerGroup(SkDevice_Compute* root, const CreateInfo&, const SkPaint*);
    ~SkDevice_ComputeLayerGroup() override;

    void drawPaint(const SkPaint& paint) override {
        this->sanityCheck();
        fRoot->drawPaint(paint);
    }

    void
    drawPoints(SkCanvas::PointMode pm, size_t s, const SkPoint pts[], const SkPaint& p) override {
        this->sanityCheck();
        fRoot->drawPoints(pm, s, pts, p);
    }

    void drawRect(const SkRect& r, const SkPaint& p) override {
        this->sanityCheck();
        fRoot->drawRect(r, p);
    }

    void drawOval(const SkRect& r, const SkPaint& p) override {
        this->sanityCheck();
        fRoot->drawOval(r, p);
    }

    void drawRRect(const SkRRect& rr, const SkPaint& p) override {
        this->sanityCheck();
        fRoot->drawRRect(rr, p);
    }

    void drawPath(const SkPath& path, const SkPaint& p, const SkMatrix* m, bool b) override {
        this->sanityCheck();
        fRoot->drawPath(path, p, m, b);
    }

    void drawText(const void* t, size_t l, SkScalar x, SkScalar y, const SkPaint& p) override {
        this->sanityCheck();
        fRoot->drawText(t, l, x, y, p);
    }

    void drawPosText(const void* t, size_t l, const SkScalar p[], int s, const SkPoint& o,
                     const SkPaint& paint) override {
        this->sanityCheck();
        fRoot->drawPosText(t, l, p, s, o, paint);
    }

    void onSave() override;
    void onRestore() override;
    void onClipRect(const SkRect& rect, SkClipOp, bool aa) override;
    void onClipRRect(const SkRRect& rrect, SkClipOp, bool aa) override;
    void onClipPath(const SkPath& path, SkClipOp, bool aa) override;
    void onClipRegion(const SkRegion& deviceRgn, SkClipOp) override;
    void onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) override;
    bool onClipIsAA() const override {
        return fRoot->onClipIsAA();
    }
    void onAsRgnClip(SkRegion* rgn) const override {
        return fRoot->onAsRgnClip(rgn);
    }
    ClipType onGetClipType() const override {
        return fRoot->onGetClipType();
    }

    void onCtmChanged() override;

    void drawBitmap(const SkBitmap&, const SkMatrix&, const SkPaint&) override {}
    void drawSprite(const SkBitmap&, int, int, const SkPaint&) override {}
    void drawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint&,
                        SkCanvas::SrcRectConstraint) override {}
    void drawDevice(SkBaseDevice*, int, int, const SkPaint&) override;
    void drawVertices(const SkVertices*, SkBlendMode, const SkPaint&) override {}
    void flush() override;

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    friend class SkDevice_Compute;

private:
    SkDevice_Compute* fRoot;

    // Save a copy of the current group id for sanity check.
    // If the sanity check fails, we're probably in the Android world where
    // multiple top-level devices can co-exist. In that case, we can no longer use the group syntax
    // and we have to create a new root-level SkDevice_Compute with an offscreen surface.
    // According to reed@, we should be able to tell whether this sanity check will fail
    // at the compile time (e.g., Chrome and Flutter never do this; Android sometimes does this).
    skc_group_id      fGroupID;

    void sanityCheck() {
#ifdef SK_DEBUG
        // We should only change the top level device's CTM.
        // Otherwise we can't use SkDevice_ComputeLayerGroup.
        SkASSERT(fGroupID == fRoot->fGroupID);

        // The root SkDevice_Compute must have an origin (0, 0) as saveLayer won't
        // ever create another SkDevice_Compute
        SkASSERT(fRoot->getOrigin() == SkIPoint::Make(0, 0));
#endif
    }
};

#endif // SK_USE_COMPUTE_LAYER_GROUP

#endif  // SK_SUPPORT_GPU_COMPUTE
#endif  // SkDevice_Compute_DEFINED
