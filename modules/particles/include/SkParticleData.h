/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleData_DEFINED
#define SkParticleData_DEFINED

#include "SkColor.h"
#include "SkPoint.h"
#include "SkRandom.h"
#include "SkReflected.h"
#include "SkRSXform.h"

/*
 *  Various structs used to communicate particle information among emitters, affectors, etc.
 */

enum SkParticleFrame {
    kWorld_ParticleFrame,     // "Up" is { 0, -1 }
    kLocal_ParticleFrame,     // "Up" is particle's heading
    kVelocity_ParticleFrame,  // "Up" is particle's direction of travel
};

static constexpr SkFieldVisitor::EnumStringMapping gParticleFrameMapping[] = {
    { kWorld_ParticleFrame,    "World" },
    { kLocal_ParticleFrame,    "Local" },
    { kVelocity_ParticleFrame, "Velocity" },
};

struct SkParticlePose {
    SkPoint  fPosition;
    SkVector fHeading;
    SkScalar fScale;

    SkRSXform asRSXform(SkPoint ofs) const {
        const float s =  fHeading.fX * fScale;
        const float c = -fHeading.fY * fScale;
        return SkRSXform::Make(c, s,
                               fPosition.fX + -c * ofs.fX +  s * ofs.fY,
                               fPosition.fY + -s * ofs.fX + -c * ofs.fY);
    }
};

struct SkParticleVelocity {
    SkVector fLinear;
    SkScalar fAngular;
};

struct SkParticleState {
    float              fAge;          // Normalized age [0, 1]
    float              fInvLifetime;  // 1 / Lifetime
    SkParticlePose     fPose;
    SkParticleVelocity fVelocity;
    SkColor4f          fColor;
    SkScalar           fFrame;        // Parameter to drawable for animated sprites, etc.
    SkRandom           fRandom;

    SkVector getFrameHeading(SkParticleFrame frame) const {
        switch (frame) {
            case kLocal_ParticleFrame:
                return fPose.fHeading;
            case kVelocity_ParticleFrame: {
                SkVector heading = fVelocity.fLinear;
                if (!heading.normalize()) {
                    heading.set(0, -1);
                }
                return heading;
            }
            case kWorld_ParticleFrame:
            default:
                return SkVector{ 0, -1 };
        }
    }
};

struct SkParticleUpdateParams {
    float fDeltaTime;
    float fEffectAge;
    int   fAgeSource;
};

/**
 * SkParticleValue selects a specific value to be used when evaluating a curve, position on a path,
 * or any other affector that needs a scalar float input. An SkParticleValue starts with a source
 * value taken from the state of the effect or particle. That can be adjusted using a scale and
 * bias, and then reduced into the desired range (typically [0, 1]) via a chosen tile mode.
 */
struct SkParticleValue {
    enum Source {
        // Either the particle or effect age, depending on spawn or update
        kAge_Source,

        kRandom_Source,
        kParticleAge_Source,
        kEffectAge_Source,
        kPositionX_Source,
        kPositionY_Source,
        kHeadingX_Source,
        kHeadingY_Source,
        kScale_Source,
        kVelocityX_Source,
        kVelocityY_Source,
        kRotation_Source,
        kColorR_Source,
        kColorG_Source,
        kColorB_Source,
        kColorA_Source,
        kSpriteFrame_Source,
    };

    enum TileMode {
        kClamp_TileMode,
        kRepeat_TileMode,
        kMirror_TileMode,
    };

    void visitFields(SkFieldVisitor* v);
    float eval(const SkParticleUpdateParams& params, SkParticleState& ps) const;

    int   fSource   = kAge_Source;
    int   fFrame    = kWorld_ParticleFrame;
    int   fTileMode = kRepeat_TileMode;

    // We map fLeft -> 0 and fRight -> 1. This is easier to work with and reason about.
    float fLeft     = 0.0f;
    float fRight    = 1.0f;

    // Cached from the above
    float fScale    = 1.0f;
    float fBias     = 0.0f;

private:
    float getSourceValue(const SkParticleUpdateParams& params, SkParticleState& ps) const;
};

#endif // SkParticleData_DEFINED
