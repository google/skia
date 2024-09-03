/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkScalar.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFeLightSource.h"

#include <cmath>

SkPoint3 SkSVGFeDistantLight::computeDirection() const {
    // Computing direction from azimuth+elevation is two 3D rotations:
    //  - Rotate [1,0,0] about y axis first (elevation)
    //  - Rotate result about z axis (azimuth)
    // Which is just the first column vector in the 3x3 matrix Rz*Ry.
    const float azimuthRad = SkDegreesToRadians(fAzimuth);
    const float elevationRad = SkDegreesToRadians(fElevation);
    const float sinAzimuth = sinf(azimuthRad), cosAzimuth = cosf(azimuthRad);
    const float sinElevation = sinf(elevationRad), cosElevation = cosf(elevationRad);
    return SkPoint3::Make(cosAzimuth * cosElevation, sinAzimuth * cosElevation, sinElevation);
}

bool SkSVGFeDistantLight::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setAzimuth(SkSVGAttributeParser::parse<SkSVGNumberType>("azimuth", n, v)) ||
           this->setElevation(SkSVGAttributeParser::parse<SkSVGNumberType>("elevation", n, v));
}

bool SkSVGFePointLight::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setX(SkSVGAttributeParser::parse<SkSVGNumberType>("x", n, v)) ||
           this->setY(SkSVGAttributeParser::parse<SkSVGNumberType>("y", n, v)) ||
           this->setZ(SkSVGAttributeParser::parse<SkSVGNumberType>("z", n, v));
}

bool SkSVGFeSpotLight::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setX(SkSVGAttributeParser::parse<SkSVGNumberType>("x", n, v)) ||
           this->setY(SkSVGAttributeParser::parse<SkSVGNumberType>("y", n, v)) ||
           this->setZ(SkSVGAttributeParser::parse<SkSVGNumberType>("z", n, v)) ||
           this->setPointsAtX(SkSVGAttributeParser::parse<SkSVGNumberType>("pointsAtX", n, v)) ||
           this->setPointsAtY(SkSVGAttributeParser::parse<SkSVGNumberType>("pointsAtY", n, v)) ||
           this->setPointsAtZ(SkSVGAttributeParser::parse<SkSVGNumberType>("pointsAtZ", n, v)) ||
           this->setSpecularExponent(
                   SkSVGAttributeParser::parse<SkSVGNumberType>("specularExponent", n, v)) ||
           this->setLimitingConeAngle(
                   SkSVGAttributeParser::parse<SkSVGNumberType>("limitingConeAngle", n, v));
}
