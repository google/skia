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

#include "experimental/ngatoy/SortKey.h"

static const int kInvalidID = -1;

//------------------------------------------------------------------------------------------------
class Cmd {
public:
    Cmd(int id) : fID(id) {}
    virtual ~Cmd() {}

    int id() const { return fID; }

    virtual SortKey getKey() = 0;

    // To generate the actual image
    virtual void execute(FakeCanvas*) const = 0;
    virtual void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const = 0;

    // To generate the expected image
    virtual void execute(SkCanvas*) const = 0;
    virtual void dump() const = 0;

protected:
    const int fID = kInvalidID;

private:
};

//------------------------------------------------------------------------------------------------
class PushCmd : public Cmd {
public:
    PushCmd() : Cmd(kInvalidID) {}

    SortKey getKey() override { SkASSERT(0); return {}; }

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas* c) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const override {
        SkASSERT(0);
    }

    void dump() const override {
        SkDebugf("%d: push", fID);
    }

protected:
private:
};

//------------------------------------------------------------------------------------------------
class PopCmd : public Cmd {
public:
    PopCmd() : Cmd(kInvalidID) {}

    SortKey getKey() override { SkASSERT(0); return {}; }

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas* c) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const override {
        SkASSERT(0);
    }

    void dump() const override {
        SkDebugf("%d: pop", fID);
    }

protected:
private:
};

//------------------------------------------------------------------------------------------------
class RectCmd : public Cmd {
public:
    RectCmd(int id, uint32_t paintersOrder, int materialID, SkIRect r, bool isTransparent, SkColor c0, SkColor c1, sk_sp<FakeMCBlob> state);

    SortKey getKey() override;

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas* c) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const override;

    void dump() const override {
        SkDebugf("%d: drawRect %d %d %d %d",
                 fID,
                 fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom);
    }

protected:

private:
    uint32_t computeZ() const;
    SkColor evalColor(int x, int y, const SkColor colors[2]) const;

    SkIRect           fRect;
    uint32_t          fPaintersOrder;
    int               fMaterialID;
    SkColor           fColors[2];
    sk_sp<FakeMCBlob> fMCState1;
};

//------------------------------------------------------------------------------------------------
class ClipCmd : public Cmd {
public:
    ClipCmd(int id, uint32_t paintersOrder, SkIRect r);
    ~ClipCmd() override { SkASSERT(fZWhenPopped != kInvalidZ); }

    SortKey getKey() override;
    uint32_t getZWhenPopped() const { SkASSERT(fZWhenPopped != kInvalidZ); return fZWhenPopped; }

    void pop(uint32_t zWhenPopped);

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas*) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const override;

    void dump() const override {
        SkDebugf("%d: clipRect %d %d %d %d",
                 fID,
                 fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom);
    }

protected:

private:
    SkIRect  fRect;
    uint32_t fPaintersOrder;
    uint32_t fZWhenPopped = kInvalidZ;
};

//------------------------------------------------------------------------------------------------

#endif // Cmds_DEFINED
