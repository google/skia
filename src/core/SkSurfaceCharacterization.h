/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurfaceCharacterization_DEFINED
#define SkSurfaceCharacterization_DEFINED

class SkSurfaceCharacterization {
public:
    SkSurfaceCharacterization()
            : fWidth(0)
            , fHeight(0)
            , fConfig(kRGBA_half_GrPixelConfig) {
    }

    void set(int width, int height, GrPixelConfig config) {
        fWidth = width;
        fHeight = height;
        fConfig = config;
    }

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    GrPixelConfig config() const { return fConfig; }

private:
    int fWidth;
    int fHeight;
    GrPixelConfig fConfig;
};

#endif
