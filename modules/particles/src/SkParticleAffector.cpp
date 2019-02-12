/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkParticleAffector.h"

#include "SkParticleData.h"
#include "SkRandom.h"

class SkDirectionalForceAffector : public SkParticleAffector {
public:
    SkDirectionalForceAffector(SkVector force = { 0.0f, 0.0f }) : fForce(force) {}

    REFLECTED(SkDirectionalForceAffector, SkParticleAffector)

    void apply(SkParticleUpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        pv.fVelocity.fLinear += fForce * params.fDeltaTime;
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Force", fForce);
    }

private:
    SkVector fForce;
};

class SkRangedForceAffector : public SkParticleAffector {
public:
    SkRangedForceAffector(const SkCurve& angle = 0.0f,
                          const SkCurve& strength = 0.0f,
                          bool bidirectional = false)
        : fAngle(angle)
        , fStrength(strength)
        , fBidirectional(bidirectional) {}

    REFLECTED(SkRangedForceAffector, SkParticleAffector)

    void apply(SkParticleUpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        float angle = fAngle.eval(params.fParticleT, *params.fStableRandom);
        SkScalar c, s = SkScalarSinCos(angle, &c);
        float strength = fStrength.eval(params.fParticleT, *params.fStableRandom);
        if (fBidirectional && params.fStableRandom->nextBool()) {
            strength = -strength;
        }
        SkVector force = { c * strength, s * strength };
        pv.fVelocity.fLinear += force * params.fDeltaTime;
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Angle", fAngle);
        v->visit("Strength", fStrength);
        v->visit("Bidirectional", fBidirectional);
    }

private:
    SkCurve fAngle;
    SkCurve fStrength;
    bool fBidirectional;
};

class SkPointForceAffector : public SkParticleAffector {
public:
    SkPointForceAffector(SkPoint point = { 0.0f, 0.0f }, SkScalar constant = 0.0f,
                         SkScalar invSquare = 0.0f)
            : fPoint(point), fConstant(constant), fInvSquare(invSquare) {}

    REFLECTED(SkPointForceAffector, SkParticleAffector)

    void apply(SkParticleUpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        SkVector toPoint = fPoint - pv.fPose.fPosition;
        SkScalar lenSquare = toPoint.dot(toPoint);
        toPoint.normalize();
        pv.fVelocity.fLinear += toPoint * (fConstant + (fInvSquare/lenSquare)) * params.fDeltaTime;
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Point", fPoint);
        v->visit("Constant", fConstant);
        v->visit("InvSquare", fInvSquare);
    }

private:
    SkPoint  fPoint;
    SkScalar fConstant;
    SkScalar fInvSquare;
};

class SkOrientAlongVelocityAffector : public SkParticleAffector {
public:
    SkOrientAlongVelocityAffector() {}

    REFLECTED(SkOrientAlongVelocityAffector, SkParticleAffector)

    void apply(SkParticleUpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        SkVector heading = pv.fVelocity.fLinear;
        if (!heading.normalize()) {
            heading.set(0, -1);
        }
        pv.fPose.fHeading = heading;
    }

    void visitFields(SkFieldVisitor*) override {}
};

class SkJitterAffector : public SkParticleAffector {
public:
    SkJitterAffector() {}

    REFLECTED(SkJitterAffector, SkParticleAffector)

    void apply(SkParticleUpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        pv.fVelocity.fLinear.fX += fX.eval(*params.fRandom);
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("X", fX);
    }

private:
    SkRangedFloat fX;
};

void SkParticleAffector::RegisterAffectorTypes() {
    REGISTER_REFLECTED(SkParticleAffector);
    REGISTER_REFLECTED(SkDirectionalForceAffector);
    REGISTER_REFLECTED(SkRangedForceAffector);
    REGISTER_REFLECTED(SkPointForceAffector);
    REGISTER_REFLECTED(SkOrientAlongVelocityAffector);
    REGISTER_REFLECTED(SkJitterAffector);
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeDirectionalForce(SkVector force) {
    return sk_sp<SkParticleAffector>(new SkDirectionalForceAffector(force));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeRangedForce(const SkCurve& angle,
                                                              const SkCurve& strength,
                                                              bool bidirectional) {
    return sk_sp<SkParticleAffector>(new SkRangedForceAffector(angle, strength, bidirectional));
}

sk_sp<SkParticleAffector> MakePointForce(SkPoint point, SkScalar constant, SkScalar invSquare) {
    return sk_sp<SkParticleAffector>(new SkPointForceAffector(point, constant, invSquare));
}

sk_sp<SkParticleAffector> MakeOrientAlongVelocity() {
    return sk_sp<SkParticleAffector>(new SkOrientAlongVelocityAffector());
}
