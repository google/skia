/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLSLCaps.h"

////////////////////////////////////////////////////////////////////////////////////////////

GrGLSLCaps::GrGLSLCaps(const GrContextOptions& options) {
    fGLSLGeneration = k330_GrGLSLGeneration;

    fDropsTileOnZeroDivide = false;
    fFBFetchSupport = false;
    fFBFetchNeedsCustomOutput = false;
    fBindlessTextureSupport = false;
    fFBFetchColorName = NULL;
    fFBFetchExtensionString = NULL;
    fAdvBlendEqInteraction = kNotSupported_AdvBlendEqInteraction;
}

SkString GrGLSLCaps::dump() const {
    SkString r = INHERITED::dump();

    static const char* kAdvBlendEqInteractionStr[] = {
        "Not Supported",
        "Automatic",
        "General Enable",
        "Specific Enables",
    };
    GR_STATIC_ASSERT(0 == kNotSupported_AdvBlendEqInteraction);
    GR_STATIC_ASSERT(1 == kAutomatic_AdvBlendEqInteraction);
    GR_STATIC_ASSERT(2 == kGeneralEnable_AdvBlendEqInteraction);
    GR_STATIC_ASSERT(3 == kSpecificEnables_AdvBlendEqInteraction);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kAdvBlendEqInteractionStr) == kLast_AdvBlendEqInteraction + 1);

    r.appendf("--- GLSL-Specific ---\n");

    r.appendf("FB Fetch Support: %s\n", (fFBFetchSupport ? "YES" : "NO"));
    r.appendf("Drops tile on zero divide: %s\n", (fDropsTileOnZeroDivide ? "YES" : "NO"));
    r.appendf("Bindless texture support: %s\n", (fBindlessTextureSupport ? "YES" : "NO"));
    r.appendf("Advanced blend equation interaction: %s\n",
              kAdvBlendEqInteractionStr[fAdvBlendEqInteraction]);
    return r;
}

