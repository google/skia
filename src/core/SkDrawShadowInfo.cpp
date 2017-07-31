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
    SkScalar ambientBlur;
    SkScalar spotBlur;
    SkScalar spotScale;
    SkPoint spotOffset;
    if (ctm.hasPerspective()) {
        // transform ambient and spot bounds into device space
        ctm.mapRect(&ambientBounds);

        // get ambient blur (in device space)
        ambientBlur = SkDrawShadowMetrics::AmbientBlurRadius(occluderZ);

        // get spot params (in device space)
        SkPoint devLightPos = SkPoint::Make(rec.fLightPos.fX, rec.fLightPos.fY);
        ctm.mapPoints(&devLightPos, 1);
        SkDrawShadowMetrics::GetSpotParams(occluderZ, devLightPos.fX, devLightPos.fY,
                                           rec.fLightPos.fZ, rec.fLightRadius,
                                           &spotBlur, &spotScale, &spotOffset);
    } else {
        SkScalar devToSrcScale = SkScalarInvert(ctm.getMinScale());

        // get ambient blur (in local space)
        SkScalar devSpaceAmbientBlur = SkDrawShadowMetrics::AmbientBlurRadius(occluderZ);
        ambientBlur = devSpaceAmbientBlur*devToSrcScale;

        // get spot params (in local space)
        SkDrawShadowMetrics::GetSpotParams(occluderZ, rec.fLightPos.fX, rec.fLightPos.fY,
                                           rec.fLightPos.fZ, rec.fLightRadius,
                                           &spotBlur, &spotScale, &spotOffset);

        // convert spot blur to local space
        spotBlur *= devToSrcScale;
    }

    // in both cases, adjust ambient and spot bounds
    SkRect spotBounds = ambientBounds;
    ambientBounds.outset(ambientBlur, ambientBlur);
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

    // if perspective, transform back to src space
    if (ctm.hasPerspective()) {
        // TODO: create tighter mapping from dev rect back to src rect
        SkMatrix inverse;
        if (ctm.invert(&inverse)) {
            inverse.mapRect(bounds);
        }
    }
}


}

