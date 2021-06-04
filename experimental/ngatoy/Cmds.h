// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Cmds_DEFINED
#define Cmds_DEFINED

class SkBitmap;
class SkCanvas;
class FakeCanvas;
class FakeMCBlob;
class SortKey;

#include "include/core/SkColor.h"
#include "include/core/SkRect.h"

#include "experimental/ngatoy/Fake.h"

class Cmd {
public:
    Cmd(int id, int materialID, sk_sp<FakeMCBlob> state)
        : fID(id)
        , fMaterialID(materialID)
        , fMCState(std::move(state)) {
    }
    virtual ~Cmd() {}

    int id() const { return fID; }

    virtual SortKey getKey() = 0;

    const FakeMCBlob* state() const { return fMCState.get(); }

    // To generate the actual image
    virtual void execute(FakeCanvas*) const = 0;
    virtual void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const = 0;

    // To generate the expected image
    virtual void execute(SkCanvas*, const FakeMCBlob* priorState) const = 0;
    virtual void dump() const = 0;

protected:
    const int         fID;
    int               fMaterialID;
    sk_sp<FakeMCBlob> fMCState;

private:
};

class RectCmd : public Cmd {
public:
    RectCmd(int id, uint32_t paintersOrder, SkIRect, const FakePaint&, sk_sp<FakeMCBlob> state);

    SortKey getKey() override;

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas* c, const FakeMCBlob* priorState) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const override;

    void dump() const override {
        SkDebugf("%d: drawRect %d %d %d %d -- %d",
                 fID,
                 fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom,
                 fPaintersOrder);
    }

protected:

private:
    uint32_t  fPaintersOrder;
    SkIRect   fRect;
    FakePaint fPaint;
};

#endif // Cmds_DEFINED
