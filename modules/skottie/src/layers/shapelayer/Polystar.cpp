/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathBuilder.h"
#include "include/private/SkTPin.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/layers/shapelayer/ShapeLayer.h"
#include "modules/sksg/include/SkSGPath.h"

namespace skottie {
namespace internal {

namespace  {

class PolystarGeometryAdapter final :
        public DiscardableAdapterBase<PolystarGeometryAdapter, sksg::Path> {
public:
    enum class Type {
        kStar, kPoly,
    };

    PolystarGeometryAdapter(const skjson::ObjectValue& jstar,
                            const AnimationBuilder* abuilder, Type t)
        : fType(t) {
        this->bind(*abuilder, jstar["pt"], fPointCount    );
        this->bind(*abuilder, jstar["p" ], fPosition      );
        this->bind(*abuilder, jstar["r" ], fRotation      );
        this->bind(*abuilder, jstar["ir"], fInnerRadius   );
        this->bind(*abuilder, jstar["or"], fOuterRadius   );
        this->bind(*abuilder, jstar["is"], fInnerRoundness);
        this->bind(*abuilder, jstar["os"], fOuterRoundness);
    }

private:
    void onSync() override {
        static constexpr int kMaxPointCount = 100000;
        const auto count = SkToUInt(SkTPin(SkScalarRoundToInt(fPointCount), 0, kMaxPointCount));
        const auto arc   = sk_ieee_float_divide(SK_ScalarPI * 2, count);

        const auto pt_on_circle = [](const SkV2& c, SkScalar r, SkScalar a) {
            return SkPoint::Make(c.x + r * std::cos(a),
                                 c.y + r * std::sin(a));
        };

        // TODO: inner/outer "roundness"?

        SkPathBuilder poly;

        auto angle = SkDegreesToRadians(fRotation - 90);
        poly.moveTo(pt_on_circle(fPosition, fOuterRadius, angle));
        poly.incReserve(fType == Type::kStar ? count * 2 : count);

        for (unsigned i = 0; i < count; ++i) {
            if (fType == Type::kStar) {
                poly.lineTo(pt_on_circle(fPosition, fInnerRadius, angle + arc * 0.5f));
            }
            angle += arc;
            poly.lineTo(pt_on_circle(fPosition, fOuterRadius, angle));
        }

        poly.close();
        this->node()->setPath(poly.detach());
    }

    const Type fType;

    Vec2Value   fPosition       = {0,0};
    ScalarValue fPointCount     = 0,
                fRotation       = 0,
                fInnerRadius    = 0,
                fOuterRadius    = 0,
                fInnerRoundness = 0,
                fOuterRoundness = 0;
};

} // namespace

sk_sp<sksg::GeometryNode> ShapeBuilder::AttachPolystarGeometry(const skjson::ObjectValue& jstar,
                                                               const AnimationBuilder* abuilder) {
    static constexpr PolystarGeometryAdapter::Type gTypes[] = {
        PolystarGeometryAdapter::Type::kStar, // "sy": 1
        PolystarGeometryAdapter::Type::kPoly, // "sy": 2
    };

    const auto type = ParseDefault<size_t>(jstar["sy"], 0) - 1;
    if (type >= SK_ARRAY_COUNT(gTypes)) {
        abuilder->log(Logger::Level::kError, &jstar, "Unknown polystar type.");
        return nullptr;
    }

    return abuilder->attachDiscardableAdapter<PolystarGeometryAdapter>
                (jstar, abuilder, gTypes[type]);
}

} // namespace internal
} // namespace skottie
