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

void SkParticleAffector::apply(SkParticleUpdateParams& params, SkParticleState ps[], int count) {
    if (fEnabled) {
        this->onApply(params, ps, count);
    }
}

void SkParticleAffector::visitFields(SkFieldVisitor* v) {
    v->visit("Enabled", fEnabled);
}

class SkLinearVelocityAffector : public SkParticleAffector {
public:
    SkLinearVelocityAffector(const SkCurve& angle = 0.0f,
                             const SkCurve& strength = 0.0f,
                             bool force = true,
                             bool local = false)
        : fAngle(angle)
        , fStrength(strength)
        , fForce(force)
        , fLocal(local) {}

    REFLECTED(SkLinearVelocityAffector, SkParticleAffector)

    void onApply(SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            float angle = fAngle.eval(ps[i].fAge, ps[i].fStableRandom);
            SkScalar c_local, s_local = SkScalarSinCos(SkDegreesToRadians(angle), &c_local);
            SkVector heading = fLocal ? ps[i].fPose.fHeading : SkVector{ 0, -1 };
            SkScalar c = heading.fX * c_local - heading.fY * s_local;
            SkScalar s = heading.fX * s_local + heading.fY * c_local;
            float strength = fStrength.eval(ps[i].fAge, ps[i].fStableRandom);
            SkVector force = { c * strength, s * strength };
            if (fForce) {
                ps[i].fVelocity.fLinear += force * params.fDeltaTime;
            } else {
                ps[i].fVelocity.fLinear = force;
            }
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
        v->visit("Force", fForce);
        v->visit("Local", fLocal);
        v->visit("Angle", fAngle);
        v->visit("Strength", fStrength);
    }

private:
    SkCurve fAngle;
    SkCurve fStrength;
    bool fForce;
    bool fLocal;
};

class SkPointForceAffector : public SkParticleAffector {
public:
    SkPointForceAffector(SkPoint point = { 0.0f, 0.0f }, SkScalar constant = 0.0f,
                         SkScalar invSquare = 0.0f)
            : fPoint(point), fConstant(constant), fInvSquare(invSquare) {}

    REFLECTED(SkPointForceAffector, SkParticleAffector)

    void onApply(SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            SkVector toPoint = fPoint - ps[i].fPose.fPosition;
            SkScalar lenSquare = toPoint.dot(toPoint);
            toPoint.normalize();
            ps[i].fVelocity.fLinear +=
                    toPoint * (fConstant + (fInvSquare / lenSquare)) * params.fDeltaTime;
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
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

    void onApply(SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            SkVector heading = ps[i].fVelocity.fLinear;
            if (!heading.normalize()) {
                heading.set(0, -1);
            }
            ps[i].fPose.fHeading = heading;
        }
    }

    void visitFields(SkFieldVisitor *v) override {
        SkParticleAffector::visitFields(v);
    }
};

class SkSizeAffector : public SkParticleAffector {
public:
    SkSizeAffector(const SkCurve& curve = 1.0f) : fCurve(curve) {}

    REFLECTED(SkSizeAffector, SkParticleAffector)

    void onApply(SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            ps[i].fPose.fScale = fCurve.eval(ps[i].fAge, ps[i].fStableRandom);
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
        v->visit("Curve", fCurve);
    }

private:
    SkCurve fCurve;
};

class SkFrameAffector : public SkParticleAffector {
public:
    SkFrameAffector(const SkCurve& curve = 1.0f) : fCurve(curve) {}

    REFLECTED(SkFrameAffector, SkParticleAffector)

    void onApply(SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            ps[i].fFrame = fCurve.eval(ps[i].fAge, ps[i].fStableRandom);
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
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

    void onApply(SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            ps[i].fColor = fCurve.eval(ps[i].fAge, ps[i].fStableRandom);
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
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
                                                                 bool force,
                                                                 bool local) {
    return sk_sp<SkParticleAffector>(new SkLinearVelocityAffector(angle, strength, force, local));
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
