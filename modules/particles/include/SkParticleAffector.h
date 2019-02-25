/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleAffector_DEFINED
#define SkParticleAffector_DEFINED

#include "SkReflected.h"

#include "SkPoint.h"

struct SkColorCurve;
struct SkCurve;
struct SkParticleState;
struct SkParticleUpdateParams;

class SkParticleAffector : public SkReflected {
public:
    REFLECTED_ABSTRACT(SkParticleAffector, SkReflected)

    void apply(SkParticleUpdateParams& params, SkParticleState& ps);
    void visitFields(SkFieldVisitor* v) override;

    static void RegisterAffectorTypes();

    static sk_sp<SkParticleAffector> MakeLinearVelocity(const SkCurve& angle,
                                                        const SkCurve& strength,
                                                        bool force,
                                                        bool local);
    static sk_sp<SkParticleAffector> MakePointForce(SkPoint point, SkScalar constant,
                                                    SkScalar invSquare);
    static sk_sp<SkParticleAffector> MakeOrientAlongVelocity();

    static sk_sp<SkParticleAffector> MakeSize(const SkCurve& curve);
    static sk_sp<SkParticleAffector> MakeFrame(const SkCurve& curve);
    static sk_sp<SkParticleAffector> MakeColor(const SkColorCurve& curve);

private:
    virtual void onApply(SkParticleUpdateParams& params, SkParticleState& ps) = 0;

    bool fEnabled = true;
};

#endif // SkParticleAffector_DEFINED
