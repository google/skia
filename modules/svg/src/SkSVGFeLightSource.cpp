/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFeLightSource.h"
#include "modules/svg/include/SkSVGValue.h"

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
