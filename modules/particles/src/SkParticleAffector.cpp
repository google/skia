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

    void apply(SkParticleUpdateParams& params, SkParticleState& ps) override {
        float angle = fAngle.eval(ps.fAge, ps.fStableRandom);
        SkScalar c, s = SkScalarSinCos(SkDegreesToRadians(angle), &c);
        float strength = fStrength.eval(ps.fAge, ps.fStableRandom);
        SkVector force = { c * strength, s * strength };
        if (fForce) {
            ps.fVelocity.fLinear += force * params.fDeltaTime;
        } else {
            ps.fVelocity.fLinear = force;
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

    void apply(SkParticleUpdateParams& params, SkParticleState& ps) override {
        SkVector toPoint = fPoint - ps.fPose.fPosition;
        SkScalar lenSquare = toPoint.dot(toPoint);
        toPoint.normalize();
        ps.fVelocity.fLinear += toPoint * (fConstant + (fInvSquare/lenSquare)) * params.fDeltaTime;
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

    void apply(SkParticleUpdateParams& params, SkParticleState& ps) override {
        SkVector heading = ps.fVelocity.fLinear;
        if (!heading.normalize()) {
            heading.set(0, -1);
        }
        ps.fPose.fHeading = heading;
    }

    void visitFields(SkFieldVisitor*) override {}
};

class SkSizeAffector : public SkParticleAffector {
public:
    SkSizeAffector(const SkCurve& curve = 1.0f) : fCurve(curve) {}

    REFLECTED(SkSizeAffector, SkParticleAffector)

    void apply(SkParticleUpdateParams& params, SkParticleState& ps) override {
        ps.fPose.fScale = fCurve.eval(ps.fAge, ps.fStableRandom);
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Curve", fCurve);
    }

private:
    SkCurve fCurve;
};

class SkFrameAffector : public SkParticleAffector {
public:
    SkFrameAffector(const SkCurve& curve = 1.0f) : fCurve(curve) {}

    REFLECTED(SkFrameAffector, SkParticleAffector)

    void apply(SkParticleUpdateParams& params, SkParticleState& ps) override {
        ps.fFrame = fCurve.eval(ps.fAge, ps.fStableRandom);
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Curve", fCurve);
    }

private:
    SkCurve fCurve;
};

class SkColorAffector : public SkParticleAffector {
public:
    SkColorAffector(const SkColorCurve& curve = SkColor4f{ 1.0f, 1.0f, 1.0f, 1.0f })
        : fCurve(curve) {}

    REFLECTED(SkColorAffector, SkParticleAffector)

    void apply(SkParticleUpdateParams& params, SkParticleState& ps) override {
        ps.fColor = fCurve.eval(ps.fAge, ps.fStableRandom);
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Curve", fCurve);
    }

private:
    SkColorCurve fCurve;
};

void SkParticleAffector::RegisterAffectorTypes() {
    REGISTER_REFLECTED(SkParticleAffector);
    REGISTER_REFLECTED(SkLinearVelocityAffector);
    REGISTER_REFLECTED(SkPointForceAffector);
    REGISTER_REFLECTED(SkOrientAlongVelocityAffector);
    REGISTER_REFLECTED(SkSizeAffector);
    REGISTER_REFLECTED(SkFrameAffector);
    REGISTER_REFLECTED(SkColorAffector);
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

sk_sp<SkParticleAffector> SkParticleAffector::MakeSize(const SkCurve& curve) {
    return sk_sp<SkParticleAffector>(new SkSizeAffector(curve));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeFrame(const SkCurve& curve) {
    return sk_sp<SkParticleAffector>(new SkFrameAffector(curve));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeColor(const SkColorCurve& curve) {
    return sk_sp<SkParticleAffector>(new SkColorAffector(curve));
}
