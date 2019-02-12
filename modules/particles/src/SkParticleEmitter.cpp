/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkParticleEmitter.h"

#include "SkContourMeasure.h"
#include "SkParticleData.h"
#include "SkRandom.h"
#include "SkTextUtils.h"

#include "sk_tool_utils.h"

class SkCircleEmitter : public SkParticleEmitter {
public:
    SkCircleEmitter(SkPoint center = { 0.0f, 0.0f }, SkScalar radius = 0.0f)
        : fCenter(center), fRadius(radius) {}

    REFLECTED(SkCircleEmitter, SkParticleEmitter)

    SkParticlePose emit(SkRandom& random) const override {
        SkVector v;
        do {
            v.fX = random.nextSScalar1();
            v.fY = random.nextSScalar1();
        } while (v.distanceToOrigin() > 1);
        SkParticlePose pose = { fCenter + (v * fRadius), v, 1.0f };
        if (!pose.fHeading.normalize()) {
            pose.fHeading.set(0, -1);
        }
        return pose;
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Center", fCenter);
        v->visit("Radius", fRadius);
    }

private:
    SkPoint  fCenter;
    SkScalar fRadius;
};

class SkLineEmitter : public SkParticleEmitter {
public:
    SkLineEmitter(SkPoint p1 = { 0.0f, 0.0f }, SkPoint p2 = { 0.0f, 0.0f }) : fP1(p1), fP2(p2) {}

    REFLECTED(SkLineEmitter, SkParticleEmitter)

    SkParticlePose emit(SkRandom& random) const override {
        SkVector localXAxis = (fP2 - fP1);
        SkParticlePose pose = { fP1 + (fP2 - fP1) * random.nextUScalar1(),
                                { localXAxis.fY, -localXAxis.fX },
                                1.0f };
        if (!pose.fHeading.normalize()) {
            pose.fHeading.set(0, -1);
        }
        return pose;
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("P1", fP1);
        v->visit("P2", fP2);
    }

private:
    SkPoint fP1;
    SkPoint fP2;
};

class SkTextEmitter : public SkParticleEmitter {
public:
    SkTextEmitter(const char* text = "", SkScalar fontSize = 96)
            : fText(text), fFontSize(fontSize) {
        this->rebuild();
    }

    REFLECTED(SkTextEmitter, SkParticleEmitter)

    SkParticlePose emit(SkRandom& random) const override {
        if (fContours.count() == 0) {
            return SkParticlePose{ { 0, 0 }, { 0, -1 }, 0.0f };
        }

        SkScalar len = random.nextRangeScalar(0, fTotalLength);
        int idx = 0;
        while (idx < fContours.count() && len > fContours[idx]->length()) {
            len -= fContours[idx++]->length();
        }
        SkParticlePose pose;
        SkVector localXAxis;
        if (!fContours[idx]->getPosTan(len, &pose.fPosition, &localXAxis)) {
            pose.fPosition = { 0, 0 };
            localXAxis = { 1, 0 };
        }
        pose.fHeading = { localXAxis.fY, -localXAxis.fX };
        pose.fScale = 1.0f;
        return pose;
    }

    void visitFields(SkFieldVisitor* v) override {
        SkString oldText = fText;
        SkScalar oldSize = fFontSize;

        v->visit("Text", fText);
        v->visit("FontSize", fFontSize);

        if (fText != oldText || fFontSize != oldSize) {
            this->rebuild();
        }
    }

private:
    SkString fText;
    SkScalar fFontSize;

    void rebuild() {
        fTotalLength = 0;
        fContours.reset();

        if (fText.isEmpty()) {
            return;
        }

        SkFont font(sk_tool_utils::create_portable_typeface());
        font.setSize(fFontSize);
        SkPath path;
        SkTextUtils::GetPath(fText.c_str(), fText.size(), kUTF8_SkTextEncoding, 0, 0, font, &path);
        fIter.reset(path, false);
        while (auto contour = fIter.next()) {
            fContours.push_back(contour);
            fTotalLength += contour->length();
        }
    }

    // Cached
    SkScalar                          fTotalLength;
    SkContourMeasureIter              fIter;
    SkTArray<sk_sp<SkContourMeasure>> fContours;
};

void SkParticleEmitter::RegisterEmitterTypes() {
    // Register types for serialization
    REGISTER_REFLECTED(SkParticleEmitter);
    REGISTER_REFLECTED(SkCircleEmitter);
    REGISTER_REFLECTED(SkLineEmitter);
    REGISTER_REFLECTED(SkTextEmitter);
}

sk_sp<SkParticleEmitter> SkParticleEmitter::MakeCircle(SkPoint center, SkScalar radius) {
    return sk_sp<SkParticleEmitter>(new SkCircleEmitter(center, radius));
}

sk_sp<SkParticleEmitter> SkParticleEmitter::MakeLine(SkPoint p1, SkPoint p2) {
    return sk_sp<SkParticleEmitter>(new SkLineEmitter(p1, p2));
}

sk_sp<SkParticleEmitter> SkParticleEmitter::MakeText(const char* text, SkScalar fontSize) {
    return sk_sp<SkParticleEmitter>(new SkTextEmitter(text, fontSize));
}
