
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLights.h"
#include "SkReadBuffer.h"

sk_sp<SkLights> SkLights::MakeFromBuffer(SkReadBuffer& buf) {
    int numLights = buf.readInt();

    Builder builder;
    for (int l = 0; l < numLights; ++l) {
        bool isAmbient = buf.readBool();
        bool isPoint = buf.readBool();

        SkColor3f color;
        if (!buf.readScalarArray(&color.fX, 3)) {
            return nullptr;
        }

        if (isAmbient) {
            builder.add(Light::MakeAmbient(color));
        } else {
            SkVector3 dirOrPos;
            if (!buf.readScalarArray(&dirOrPos.fX, 3)) {
                return nullptr;
            }
            SkScalar intensity = 0.0f;
            if (isPoint) {
                intensity = buf.readScalar();
            }

            sk_sp<SkImage> depthMap;
            bool hasShadowMap = buf.readBool();
            if (hasShadowMap) {
                if (!(depthMap = buf.readImage())) {
                    return nullptr;
                }
            }

            if (isPoint) {
                Light light = Light::MakePoint(color, dirOrPos, intensity);
                light.setShadowMap(depthMap);
                builder.add(light);
            } else {
                Light light = Light::MakeDirectional(color, dirOrPos);
                light.setShadowMap(depthMap);
                builder.add(light);
            }
        }
    }

    return builder.finish();
}

void SkLights::flatten(SkWriteBuffer& buf) const {

    buf.writeInt(this->numLights());
    for (int l = 0; l < this->numLights(); ++l) {
        const Light& light = this->light(l);

        bool isAmbient = Light::kAmbient_LightType == light.type();
        bool isPoint = Light::kPoint_LightType == light.type();

        buf.writeBool(isAmbient);
        buf.writeBool(isPoint);
        buf.writeScalarArray(&light.color().fX, 3);
        if (!isAmbient) {
            if (isPoint) {
                buf.writeScalarArray(&light.pos().fX, 3);
                buf.writeScalar(light.intensity());
            } else {
                buf.writeScalarArray(&light.dir().fX, 3);
            }
            bool hasShadowMap = light.getShadowMap() != nullptr;
            buf.writeBool(hasShadowMap);
            if (hasShadowMap) {
                buf.writeImage(light.getShadowMap());
            }
        }
    }
}
