/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FuzzCommon_DEFINED
#define FuzzCommon_DEFINED

#include "fuzz/Fuzz.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkTArray.h"

class SkData;

// allows some float values for path points
void FuzzNicePath(Fuzz* fuzz, SkPathBuilder*, int maxOps);

static inline SkPath FuzzNicePath(Fuzz* fuzz, int maxOps) {
    SkPathBuilder builder;
    FuzzNicePath(fuzz, &builder, maxOps);
    return builder.detach();
}

// allows all float values for path points
SkPath FuzzEvilPath(Fuzz* fuzz, int last_verb);

void FuzzNiceRRect(Fuzz* fuzz, SkRRect* rr);

void FuzzNiceMatrix(Fuzz* fuzz, SkMatrix* m);

void FuzzNiceRegion(Fuzz* fuzz, SkRegion* region, int maxN);

void FuzzCreateValidInputsForRuntimeEffect(
        SkRuntimeEffect* effect,
        sk_sp<SkData>& uniformBytes,
        skia_private::TArray<SkRuntimeEffect::ChildPtr>& children);

#endif

