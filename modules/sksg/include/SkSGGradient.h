/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGGradient_DEFINED
#define SkSGGradient_DEFINED

#include "modules/sksg/include/SkSGRenderEffect.h"

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"

#include <vector>

namespace sksg {

/**
 * Gradient base class.
 */
class Gradient : public Shader {
public:
    struct ColorStop {
        SkScalar fPosition;
        SkColor  fColor;

        bool operator==(const ColorStop& other) const {
            return fPosition == other.fPosition && fColor == other.fColor;
        }
    };

    SG_ATTRIBUTE(ColorStops, std::vector<ColorStop>, fColorStops)
    SG_ATTRIBUTE(TileMode  , SkTileMode            , fTileMode  )

protected:
    sk_sp<SkShader> onRevalidateShader() final;

    virtual sk_sp<SkShader> onMakeShader(const std::vector<SkColor>& colors,
                                         const std::vector<SkScalar>& positions) const = 0;

protected:
    Gradient() = default;

private:
    std::vector<ColorStop> fColorStops;
    SkTileMode             fTileMode = SkTileMode::kClamp;

    using INHERITED = Shader;
};

class LinearGradient final : public Gradient {
public:
    static sk_sp<LinearGradient> Make() {
        return sk_sp<LinearGradient>(new LinearGradient());
    }

    SG_ATTRIBUTE(StartPoint, SkPoint, fStartPoint)
    SG_ATTRIBUTE(EndPoint  , SkPoint, fEndPoint  )

protected:
    sk_sp<SkShader> onMakeShader(const std::vector<SkColor>& colors,
                                 const std::vector<SkScalar>& positions) const override;

private:
    LinearGradient() = default;

    SkPoint fStartPoint = SkPoint::Make(0, 0),
            fEndPoint   = SkPoint::Make(0, 0);

    using INHERITED = Gradient;
};

class RadialGradient final : public Gradient {
public:
    static sk_sp<RadialGradient> Make() {
        return sk_sp<RadialGradient>(new RadialGradient());
    }

    SG_ATTRIBUTE(StartCenter, SkPoint , fStartCenter)
    SG_ATTRIBUTE(EndCenter  , SkPoint , fEndCenter  )
    SG_ATTRIBUTE(StartRadius, SkScalar, fStartRadius)
    SG_ATTRIBUTE(EndRadius  , SkScalar, fEndRadius  )

protected:
    sk_sp<SkShader> onMakeShader(const std::vector<SkColor>& colors,
                                 const std::vector<SkScalar>& positions) const override;

private:
    RadialGradient() = default;

    SkPoint  fStartCenter = SkPoint::Make(0, 0),
             fEndCenter   = SkPoint::Make(0, 0);
    SkScalar fStartRadius = 0,
             fEndRadius   = 0;

    using INHERITED = Gradient;
};

} // namespace sksg

#endif // SkSGGradient_DEFINED
