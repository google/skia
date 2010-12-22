/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrSamplerState_DEFINED
#define GrSamplerState_DEFINED

#include "GrTypes.h"

class GrSamplerState {
public:
    enum SampleMode {
        kNormal_SampleMode,     //!< sample color directly
        kAlphaMod_SampleMode,   //!< modulate with alpha only
        kRadial_SampleMode,     //!< treat as radial gradient
        kRadial2_SampleMode,    //!< treat as 2-point radial gradient
        kSweep_SampleMode,      //!< treat as sweep gradient
    };

    /**
     * Describes how a texture is sampled when coordinates are outside the
     * texture border
     */
    enum WrapMode {
        kClamp_WrapMode,
        kRepeat_WrapMode,
        kMirror_WrapMode
    };

    /**
     *  Default sampler state is set to kClamp and no-filter
     */
    GrSamplerState() {
        this->setClampNoFilter();
    }

    GrSamplerState(bool filter) {
        fWrapX = kClamp_WrapMode;
        fWrapY = kClamp_WrapMode;
        fSampleMode = kNormal_SampleMode;
        fFilter = filter;
    }
    
    GrSamplerState(WrapMode wx, WrapMode wy, bool filter) {
        fWrapX = wx;
        fWrapY = wy;
        fSampleMode = kNormal_SampleMode;
        fFilter = filter;
    }
    
    GrSamplerState(WrapMode wx, WrapMode wy, SampleMode sample, bool filter) {
        fWrapX = wx;
        fWrapY = wy;
        fSampleMode = sample;
        fFilter = filter;
    }
    
    WrapMode getWrapX() const { return fWrapX; }
    WrapMode getWrapY() const { return fWrapY; }
    SampleMode getSampleMode() const { return fSampleMode; }
    bool isFilter() const { return fFilter; }

    bool isGradient() const {
        return  kRadial_SampleMode == fSampleMode ||
                kRadial2_SampleMode == fSampleMode ||
                kSweep_SampleMode == fSampleMode;
    }

    void setWrapX(WrapMode mode) { fWrapX = mode; }
    void setWrapY(WrapMode mode) { fWrapY = mode; }
    void setSampleMode(SampleMode mode) { fSampleMode = mode; }
    void setFilter(bool filter) { fFilter = filter; }

    void setClampNoFilter() {
        fWrapX = kClamp_WrapMode;
        fWrapY = kClamp_WrapMode;
        fSampleMode = kNormal_SampleMode;
        fFilter = false;
    }

    GrScalar getRadial2CenterX1() const { return fRadial2CenterX1; }
    GrScalar getRadial2Radius0() const { return fRadial2Radius0; }
    bool     isRadial2PosRoot() const { return fRadial2PosRoot; }

    /**
     * Sets the parameters for kRadial2_SampleMode. The texture 
     * matrix must be set so that the first point is at (0,0) and the second 
     * point lies on the x-axis. The second radius minus the first is 1 unit.
     * The additional parameters to define the gradient are specified by this
     * function.
     */
    void setRadial2Params(GrScalar centerX1, GrScalar radius0, bool posRoot) {
        fRadial2CenterX1 = centerX1;
        fRadial2Radius0 = radius0;
        fRadial2PosRoot = posRoot;
    }

    static const GrSamplerState& ClampNoFilter() {
        return gClampNoFilter;
    }

private:
    WrapMode    fWrapX;
    WrapMode    fWrapY;
    SampleMode  fSampleMode;
    bool        fFilter;

    // these are undefined unless fSampleMode == kRadial2_SampleMode
    GrScalar    fRadial2CenterX1;
    GrScalar    fRadial2Radius0;
    bool        fRadial2PosRoot;

    static const GrSamplerState gClampNoFilter;
};

#endif

