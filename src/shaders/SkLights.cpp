
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformer.h"
#include "SkLights.h"
#include "SkReadBuffer.h"

sk_sp<SkLights> SkLights::MakeFromBuffer(SkReadBuffer& buf) {
    Builder builder;

    SkColor3f ambColor;
    if (!buf.readScalarArray(&ambColor.fX, 3)) {
        return nullptr;
    }

    builder.setAmbientLightColor(ambColor);

    int numLights = buf.readInt();

    for (int l = 0; l < numLights; ++l) {
        bool isPoint = buf.readBool();

        SkColor3f color;
        if (!buf.readScalarArray(&color.fX, 3)) {
            return nullptr;
        }

        SkVector3 dirOrPos;
        if (!buf.readScalarArray(&dirOrPos.fX, 3)) {
            return nullptr;
        }

        if (isPoint) {
            SkScalar intensity;
            intensity = buf.readScalar();
            Light light = Light::MakePoint(color, dirOrPos, intensity);
            builder.add(light);
        } else {
            Light light = Light::MakeDirectional(color, dirOrPos);
            builder.add(light);
        }
    }

    return builder.finish();
}

static SkColor3f xform_color(const SkColor3f& color, SkColorSpaceXformer* xformer) {
    SkColor origColor = SkColorSetARGB(0xFF,
                                       SkScalarRoundToInt(color.fX * 255.0f),
                                       SkScalarRoundToInt(color.fY * 255.0f),
                                       SkScalarRoundToInt(color.fZ * 255.0f));
    SkColor xformedColor = xformer->apply(origColor);
    return SkColor3f::Make(SkIntToScalar(SkGetPackedR32(xformedColor)) * (1/255.0f),
                           SkIntToScalar(SkGetPackedG32(xformedColor)) * (1/255.0f),
                           SkIntToScalar(SkGetPackedB32(xformedColor)) * (1/255.0f));
}

sk_sp<SkLights> SkLights::makeColorSpace(SkColorSpaceXformer* xformer) const {
    SkLights::Builder builder;
    for (int i = 0; i < this->numLights(); i++) {
        Light light(fLights[i].type(), xform_color(fLights[i].color(), xformer),
                    fLights[i].fDirOrPos, fLights[i].fIntensity);
        builder.add(light);
    }
    builder.setAmbientLightColor(xform_color(fAmbientLightColor, xformer));
    return builder.finish();
}

void SkLights::flatten(SkWriteBuffer& buf) const {
    buf.writeScalarArray(&this->ambientLightColor().fX, 3);

    buf.writeInt(this->numLights());
    for (int l = 0; l < this->numLights(); ++l) {
        const Light& light = this->light(l);

        bool isPoint = Light::kPoint_LightType == light.type();

        buf.writeBool(isPoint);
        buf.writeScalarArray(&light.color().fX, 3);
        buf.writeScalarArray(&light.dir().fX, 3);

        if (isPoint) {
            buf.writeScalar(light.intensity());
        }
    }
}
