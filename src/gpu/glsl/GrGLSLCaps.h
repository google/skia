/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLSLCaps_DEFINED
#define GrGLSLCaps_DEFINED

#include "GrCaps.h"
#include "GrGLSL.h"

class GrGLSLCaps : public GrShaderCaps {
public:
    

    /**
    * Indicates how GLSL must interact with advanced blend equations. The KHR extension requires
    * special layout qualifiers in the fragment shader.
    */
    enum AdvBlendEqInteraction {
        kNotSupported_AdvBlendEqInteraction,     //<! No _blend_equation_advanced extension
        kAutomatic_AdvBlendEqInteraction,        //<! No interaction required
        kGeneralEnable_AdvBlendEqInteraction,    //<! layout(blend_support_all_equations) out
        kSpecificEnables_AdvBlendEqInteraction,  //<! Specific layout qualifiers per equation

        kLast_AdvBlendEqInteraction = kSpecificEnables_AdvBlendEqInteraction
    };

    /**
     * Initializes the GrGLSLCaps to a default set of features
     */
    GrGLSLCaps(const GrContextOptions&);

    /**
     * Some helper functions for encapsulating various extensions to read FB Buffer on openglES
     *
     * TODO(joshualitt) On desktop opengl 4.2+ we can achieve something similar to this effect
     */
    bool fbFetchSupport() const { return fFBFetchSupport; }

    bool fbFetchNeedsCustomOutput() const { return fFBFetchNeedsCustomOutput; }

    bool bindlessTextureSupport() const { return fBindlessTextureSupport; }

    const char* fbFetchColorName() const { return fFBFetchColorName; }

    const char* fbFetchExtensionString() const { return fFBFetchExtensionString; }

    bool dropsTileOnZeroDivide() const { return fDropsTileOnZeroDivide; }

    AdvBlendEqInteraction advBlendEqInteraction() const { return fAdvBlendEqInteraction; }

    bool mustEnableAdvBlendEqs() const {
        return fAdvBlendEqInteraction >= kGeneralEnable_AdvBlendEqInteraction;
    }

    bool mustEnableSpecificAdvBlendEqs() const {
        return fAdvBlendEqInteraction == kSpecificEnables_AdvBlendEqInteraction;
    }
    
    bool mustDeclareFragmentShaderOutput() const {
        return fGLSLGeneration > k110_GrGLSLGeneration;
    }

    GrGLSLGeneration generation() const { return fGLSLGeneration; }

    /**
    * Returns a string containing the caps info.
    */
    SkString dump() const override;

private:
    GrGLSLGeneration fGLSLGeneration;
    
    bool fDropsTileOnZeroDivide : 1;
    bool fFBFetchSupport : 1;
    bool fFBFetchNeedsCustomOutput : 1;
    bool fBindlessTextureSupport : 1;

    const char* fFBFetchColorName;
    const char* fFBFetchExtensionString;

    AdvBlendEqInteraction fAdvBlendEqInteraction;

    friend class GrGLCaps;  // For initialization.

    typedef GrShaderCaps INHERITED;
};


#endif
