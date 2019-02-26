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

constexpr SkFieldVisitor::EnumStringMapping gParticleFrameMapping[] = {
    { kWorld_ParticleFrame,    "World" },
    { kLocal_ParticleFrame,    "Local" },
    { kVelocity_ParticleFrame, "Velocity" },
};

void SkParticleAffector::apply(SkParticleUpdateParams& params, SkParticleState ps[], int count) {
    if (fEnabled) {
        this->onApply(params, ps, count);
    }
}

void SkParticleAffector::visitFields(SkFieldVisitor* v) {
    v->visit("Enabled", fEnabled);
}

static inline SkVector get_heading(const SkParticleState& ps, SkParticleFrame frame) {
    switch (frame) {
        case kLocal_ParticleFrame:
            return ps.fPose.fHeading;
        case kVelocity_ParticleFrame: {
            SkVector heading = ps.fVelocity.fLinear;
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

class SkLinearVelocityAffector : public SkParticleAffector {
public:
    SkLinearVelocityAffector(const SkCurve& angle = 0.0f,
                             const SkCurve& strength = 0.0f,
                             bool force = true,
                             SkParticleFrame frame = kWorld_ParticleFrame)
        : fAngle(angle)
        , fStrength(strength)
        , fForce(force)
        , fFrame(frame) {}

    REFLECTED(SkLinearVelocityAffector, SkParticleAffector)

    void onApply(SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            float angle = fAngle.eval(ps[i].fAge, ps[i].fRandom);
            SkScalar c_local, s_local = SkScalarSinCos(SkDegreesToRadians(angle), &c_local);
            SkVector heading = get_heading(ps[i], static_cast<SkParticleFrame>(fFrame));
            SkScalar c = heading.fX * c_local - heading.fY * s_local;
            SkScalar s = heading.fX * s_local + heading.fY * c_local;
            float strength = fStrength.eval(ps[i].fAge, ps[i].fRandom);
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
        v->visit("Frame", fFrame, gParticleFrameMapping, SK_ARRAY_COUNT(gParticleFrameMapping));
        v->visit("Angle", fAngle);
        v->visit("Strength", fStrength);
    }

private:
    SkCurve fAngle;
    SkCurve fStrength;
    bool fForce;
    int  fFrame;
};

class SkAngularVelocityAffector : public SkParticleAffector {
public:
    SkAngularVelocityAffector(const SkCurve& strength = 0.0f, bool force = true)
        : fStrength(strength)
        , fForce(force) {}

    REFLECTED(SkAngularVelocityAffector, SkParticleAffector)

    void onApply(SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            float strength = fStrength.eval(ps[i].fAge, ps[i].fRandom);
            if (fForce) {
                ps[i].fVelocity.fAngular += strength * params.fDeltaTime;
            } else {
                ps[i].fVelocity.fAngular = strength;
            }
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
        v->visit("Force", fForce);
        v->visit("Strength", fStrength);
    }

private:
    SkCurve fStrength;
    bool    fForce;
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

class SkOrientationAffector : public SkParticleAffector {
public:
    SkOrientationAffector(const SkCurve& angle = 0.0f,
                          SkParticleFrame frame = kLocal_ParticleFrame)
        : fAngle(angle)
        , fFrame(frame) {}

    REFLECTED(SkOrientationAffector, SkParticleAffector)

    void onApply(SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            float angle = fAngle.eval(ps[i].fAge, ps[i].fRandom);
            SkScalar c_local, s_local = SkScalarSinCos(SkDegreesToRadians(angle), &c_local);
            SkVector heading = get_heading(ps[i], static_cast<SkParticleFrame>(fFrame));
            ps[i].fPose.fHeading.set(heading.fX * c_local - heading.fY * s_local,
                                     heading.fX * s_local + heading.fY * c_local);
        }
    }

    void visitFields(SkFieldVisitor *v) override {
        SkParticleAffector::visitFields(v);
        v->visit("Frame", fFrame, gParticleFrameMapping, SK_ARRAY_COUNT(gParticleFrameMapping));
        v->visit("Angle", fAngle);
    }

private:
    SkCurve fAngle;
    int     fFrame;
};

class SkSizeAffector : public SkParticleAffector {
public:
    SkSizeAffector(const SkCurve& curve = 1.0f) : fCurve(curve) {}

    REFLECTED(SkSizeAffector, SkParticleAffector)

    void onApply(SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            ps[i].fPose.fScale = fCurve.eval(ps[i].fAge, ps[i].fRandom);
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
            ps[i].fFrame = fCurve.eval(ps[i].fAge, ps[i].fRandom);
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
            ps[i].fColor = fCurve.eval(ps[i].fAge, ps[i].fRandom);
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
    REGISTER_REFLECTED(SkAngularVelocityAffector);
    REGISTER_REFLECTED(SkPointForceAffector);
    REGISTER_REFLECTED(SkOrientationAffector);
    REGISTER_REFLECTED(SkSizeAffector);
    REGISTER_REFLECTED(SkFrameAffector);
    REGISTER_REFLECTED(SkColorAffector);
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeLinearVelocity(const SkCurve& angle,
                                                                 const SkCurve& strength,
                                                                 bool force,
                                                                 SkParticleFrame frame) {
    return sk_sp<SkParticleAffector>(new SkLinearVelocityAffector(angle, strength, force, frame));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeAngularVelocity(const SkCurve& strength,
                                                                  bool force) {
    return sk_sp<SkParticleAffector>(new SkAngularVelocityAffector(strength, force));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakePointForce(SkPoint point, SkScalar constant,
                                                             SkScalar invSquare) {
    return sk_sp<SkParticleAffector>(new SkPointForceAffector(point, constant, invSquare));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeOrientation(const SkCurve& angle,
                                                              SkParticleFrame frame) {
    return sk_sp<SkParticleAffector>(new SkOrientationAffector(angle, frame));
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
