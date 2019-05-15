/*
 * Copyright 2019 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"

class SkTextShadow {
  public:
    SkColor fColor = SK_ColorBLACK;
    SkPoint fOffset;
    double fBlurRadius = 0.0;

    SkTextShadow();

    SkTextShadow(SkColor color, SkPoint offset, double blurRadius);

    bool operator==(const SkTextShadow& other) const;

    bool operator!=(const SkTextShadow& other) const;

    bool hasShadow() const;
};
