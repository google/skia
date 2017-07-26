/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDrawShadowInfo.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkRect.h"

namespace SkDrawShadowMetrics {

static SkScalar compute_z(SkScalar x, SkScalar y, const SkPoint3& params) {
    return x*params.fX + y*params.fY + params.fZ;
}

void GetLocalBounds(const SkPath& path, const SkDrawShadowRec& rec, const SkMatrix& ctm,
                    SkRect* bounds) {
    SkRect ambientBounds = path.getBounds();
    SkScalar devToSrcScale;
    if (ctm.isScaleTranslate()) {
        devToSrcScale = SkScalarInvert(ctm[SkMatrix::kMScaleX]);
    } else {
        SkRect bounds = ambientBounds;
        ctm.mapRect(&bounds);
        devToSrcScale = SkTMax(ambientBounds.width() / bounds.width(),
                               ambientBounds.height() / bounds.height());
    }
    SkScalar occluderZ;
    if (SkScalarNearlyZero(rec.fZPlaneParams.fX) && SkScalarNearlyZero(rec.fZPlaneParams.fY)) {
        occluderZ = rec.fZPlaneParams.fZ;
    } else {
        occluderZ = compute_z(ambientBounds.fLeft, ambientBounds.fTop, rec.fZPlaneParams);
        occluderZ = SkTMax(occluderZ, compute_z(ambientBounds.fRight, ambientBounds.fTop,
                                                rec.fZPlaneParams));
        occluderZ = SkTMax(occluderZ, compute_z(ambientBounds.fLeft, ambientBounds.fBottom,
                                                rec.fZPlaneParams));
        occluderZ = SkTMax(occluderZ, compute_z(ambientBounds.fRight, ambientBounds.fBottom,
                                                rec.fZPlaneParams));
    }

    // compute ambient bounds (in local space)
    SkScalar devSpaceAmbientBlur = SkDrawShadowMetrics::AmbientBlurRadius(occluderZ);
    SkScalar srcSpaceAmbientBlur = devSpaceAmbientBlur*devToSrcScale;
    ambientBounds.outset(srcSpaceAmbientBlur, srcSpaceAmbientBlur);

    // compute spot bounds (in local space)
    SkScalar spotBlur;
    SkScalar spotScale;
    SkPoint spotOffset;
    SkDrawShadowMetrics::GetSpotParams(occluderZ, rec.fLightPos.fX, rec.fLightPos.fY,
                                       rec.fLightPos.fZ, rec.fLightRadius,
                                       &spotBlur, &spotScale, &spotOffset);

    // convert spot blur to local space
    spotBlur *= devToSrcScale;

    SkRect spotBounds = path.getBounds();
    spotBounds.fLeft *= spotScale;
    spotBounds.fTop *= spotScale;
    spotBounds.fRight *= spotScale;
    spotBounds.fBottom *= spotScale;
    spotBounds.offset(spotOffset.fX, spotOffset.fY);
    spotBounds.outset(spotBlur, spotBlur);

    // merge bounds
    *bounds = ambientBounds;
    bounds->join(spotBounds);
    // outset a bit to account for floating point error
    bounds->outset(1, 1);
}

}

