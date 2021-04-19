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
    Cmd(int id) : fID(id) {}

    int id() const { return fID; }

    // To generate the actual image
    virtual void execute(FakeCanvas*) const = 0;
    virtual void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const = 0;

    // To generate the expected image
    virtual void execute(SkCanvas*) const = 0;
    virtual void dump() const = 0;

protected:
    const int fID;

private:
};

class RectCmd : public Cmd {
public:
    RectCmd(int id, SkIRect r, SkColor c) : Cmd(id), fRect(r), fColor(c) {}

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
    SkColor fColor;
};

#endif // Cmds_DEFINED
