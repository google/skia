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
    SkRangedForceAffector()
        : fAngle(0.0f)
        , fStrength(0.0f)
        , fBidirectional(false) {}

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

static SkVector up_to_right(SkVector up) {
    return { -up.fY, up.fX };
}

class SkOrientAlongVelocityAffector : public SkParticleAffector {
public:
    SkOrientAlongVelocityAffector() {}

    REFLECTED(SkOrientAlongVelocityAffector, SkParticleAffector)

    void apply(SkParticleUpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        SkVector up = pv.fVelocity.fLinear;
        if (!up.normalize()) {
            up.set(0, -1);
        }
        pv.fPose.fRight = up_to_right(up);
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
