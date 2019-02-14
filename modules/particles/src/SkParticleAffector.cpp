/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkParticleAffector.h"

#include "SkCurve.h"
#include "SkParticleData.h"
#include "SkRandom.h"

class SkLinearVelocityAffector : public SkParticleAffector {
public:
    SkLinearVelocityAffector(const SkCurve& angle = 0.0f,
                             const SkCurve& strength = 0.0f,
                             bool force = true)
        : fAngle(angle)
        , fStrength(strength)
        , fForce(force) {}

    REFLECTED(SkLinearVelocityAffector, SkParticleAffector)

    void apply(SkParticleUpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        float angle = fAngle.eval(params.fParticleT, *params.fStableRandom);
        SkScalar c, s = SkScalarSinCos(SkDegreesToRadians(angle), &c);
        float strength = fStrength.eval(params.fParticleT, *params.fStableRandom);
        SkVector force = { c * strength, s * strength };
        if (fForce) {
            pv.fVelocity.fLinear += force * params.fDeltaTime;
        } else {
            pv.fVelocity.fLinear = force;
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Force", fForce);
        v->visit("Angle", fAngle);
        v->visit("Strength", fStrength);
    }

private:
    SkCurve fAngle;
    SkCurve fStrength;
    bool fForce;
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

class SkSizeAffector : public SkParticleAffector {
public:
    SkSizeAffector(const SkCurve& curve = 1.0f) : fCurve(curve) {}

    REFLECTED(SkSizeAffector, SkParticleAffector)

    void apply(SkParticleUpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        pv.fPose.fScale = fCurve.eval(params.fParticleT, *params.fStableRandom);
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Curve", fCurve);
    }

private:
    SkCurve fCurve;
};

void SkParticleAffector::RegisterAffectorTypes() {
    REGISTER_REFLECTED(SkParticleAffector);
    REGISTER_REFLECTED(SkLinearVelocityAffector);
    REGISTER_REFLECTED(SkPointForceAffector);
    REGISTER_REFLECTED(SkOrientAlongVelocityAffector);
    REGISTER_REFLECTED(SkSizeAffector);
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeLinearVelocity(const SkCurve& angle,
                                                                 const SkCurve& strength,
                                                                 bool force) {
    return sk_sp<SkParticleAffector>(new SkLinearVelocityAffector(angle, strength, force));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakePointForce(SkPoint point, SkScalar constant,
                                                             SkScalar invSquare) {
    return sk_sp<SkParticleAffector>(new SkPointForceAffector(point, constant, invSquare));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeOrientAlongVelocity() {
    return sk_sp<SkParticleAffector>(new SkOrientAlongVelocityAffector());
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeSizeAffector(const SkCurve& curve) {
    return sk_sp<SkParticleAffector>(new SkSizeAffector(curve));
}
