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

//------------------------------------------------------------------------------------------------
class Cmd {
public:
    Cmd(SortKey k, int id, int materialID, sk_sp<FakeMCBlob> state)
        : fKey(k)
        , fID(id)
        , fMaterialID(materialID)
        , fMCState(std::move(state)) {
    }
    virtual ~Cmd() {}

    SortKey key() const { return fKey; }
    int id() const { return fID; }
    const FakeMCBlob* state() const { return fMCState.get(); }

    // To generate the actual image
    virtual void execute(FakeCanvas*) const = 0;
    virtual void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const = 0;

    // To generate the expected image
    virtual void execute(SkCanvas*) const = 0;
    virtual void dump() const = 0;

protected:
    SkColor evalColor(int x, int y, const SkColor colors[2]) const;

    SortKey           fKey;
    const int         fID;
    int               fMaterialID;
    sk_sp<FakeMCBlob> fMCState;

private:
};

//------------------------------------------------------------------------------------------------
class PushCmd : public Cmd {
public:
    PushCmd(int id) : Cmd({}, id, kInvalidMat, nullptr) {}

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas* c) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const override {
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
    PopCmd(int id) : Cmd({}, id, kInvalidMat, nullptr) {}

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas* c) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const override {
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
    RectCmd(SortKey k, int id, int materialID, SkIRect r, SkColor c0, SkColor c1, sk_sp<FakeMCBlob> state = nullptr);

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

//------------------------------------------------------------------------------------------------
class ClipCmd : public Cmd {
public:
    ClipCmd(SortKey k, int id, SkIRect r);

    void popAndLock(uint32_t z) {
        fKey = SortKey(false, 0, z, kInvalidMat);
    }

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas* c) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const override;

    void dump() const override {
        SkDebugf("%d: clipRect %d %d %d %d",
                 fID,
                 fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom);
    }

protected:

private:
    SkIRect fRect;
};

//------------------------------------------------------------------------------------------------

#endif // Cmds_DEFINED
