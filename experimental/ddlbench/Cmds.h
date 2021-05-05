// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Cmds_DEFINED
#define Cmds_DEFINED

class SkBitmap;
class SkCanvas;
class FakeCanvas;

#include "include/core/SkColor.h"
#include "include/core/SkRect.h"

class Cmd {
public:
    Cmd(int id, int materialID) : fID(id), fMaterialID(materialID) {}

    int id() const { return fID; }

    // To generate the actual image
    virtual void execute(FakeCanvas*) const = 0;
    virtual void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const = 0;

    // To generate the expected image
    virtual void execute(SkCanvas*) const = 0;
    virtual void dump() const = 0;

protected:
    static SkColor blend(float t, SkColor c0, SkColor c1);

    SkColor evalColor(int x, int y, const SkColor colors[2]) const {
        switch (fMaterialID) {
            case 1: return colors[0];
            case 2: {
                float t = SK_ScalarRoot2Over2 * x + SK_ScalarRoot2Over2 * y;
                t /= SK_ScalarSqrt2 * 256.0f;
                return blend(t, colors[0], colors[1]);
            }
            case 3: {
                x -= 128;
                y -= 128;
                float dist = sqrt(x*x + y*y) / 128.0f;
                if (dist > 1.0f) {
                    return colors[0];
                } else {
                    return blend(dist, colors[0], colors[1]);
                }
            }
        }
        SkUNREACHABLE;
    }

    const int fID;
    int       fMaterialID;

private:
};

class RectCmd : public Cmd {
public:
    RectCmd(int id, int materialID, SkIRect r, SkColor c0, SkColor c1)
        : Cmd(id, materialID)
        , fRect(r) {
        fColors[0] = c0;
        fColors[1] = c1;
    }

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas* c) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const override;

    void dump() const override {
        SkDebugf("%d: drawRect %d %d %d %d",
                 fID,
                 fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom);
    }

protected:

private:
    SkIRect fRect;
    SkColor fColors[2];
};

#endif // Cmds_DEFINED
