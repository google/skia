/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#ifndef TestingSpecialImageAccess_DEFINED
#define TestingSpecialImageAccess_DEFINED

class TestingSpecialImageAccess {
public:
    static const SkIRect& Subset(const SkSpecialImage* img) {
        return img->subset();
    }
};

#endif
