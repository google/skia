// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Cmds_DEFINED
#define Cmds_DEFINED

class SkBitmap;
class SkCanvas;
class FakeCanvas;
class FakeMCBlob;

#include "include/core/SkColor.h"
#include "include/core/SkRect.h"

class Cmd {
public:
    Cmd(int id, int materialID, sk_sp<FakeMCBlob> state)
        : fID(id)
        , fMaterialID(materialID)
        , fMCState(std::move(state)) {
    }
    virtual ~Cmd() {}

    int id() const { return fID; }
    const FakeMCBlob* state() const { return fMCState.get(); }

    // To generate the actual image
    virtual void execute(FakeCanvas*) const = 0;
    virtual void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const = 0;

    // To generate the expected image
    virtual void execute(SkCanvas*, const FakeMCBlob* priorState) const = 0;
    virtual void dump() const = 0;

protected:
    SkColor evalColor(int x, int y, const SkColor colors[2]) const;

    const int         fID;
    int               fMaterialID;
    sk_sp<FakeMCBlob> fMCState;

private:
};

class RectCmd : public Cmd {
public:
    RectCmd(int id, int materialID, SkIRect r, SkColor c0, SkColor c1, sk_sp<FakeMCBlob> state = nullptr);

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas* c, const FakeMCBlob* priorState) const override;
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
