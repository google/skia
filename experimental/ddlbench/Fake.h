// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Fake_DEFINED
#define Fake_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkTypes.h"

#include <vector>

class Cmd;
class SkBitmap;

class Key {
public:
    static const uint32_t kNumDepthBits = 3;
    static const uint32_t kDepthMask = (0x1 << kNumDepthBits) - 1;
    static const uint32_t kDepthShift = 4;

    static const uint32_t kNumMaterialBits = 4;
    static const uint32_t kMaterialMask = (0x1 << kNumMaterialBits) - 1;
    static const uint32_t kMaterialShift = 0;

    Key() : fKey(0) {}
    Key(uint32_t depth, uint32_t material) {
        SkASSERT(!(depth & ~kDepthMask));
        SkASSERT(!(material & ~kMaterialMask));

        fKey = (depth & kDepthMask) << kDepthShift |
               (material & kMaterialMask) << kMaterialShift;
    }

    uint32_t getZ() const {
        return (fKey >> kDepthShift) & kDepthMask;
    }

    void dump() const {
        SkDebugf("depth: %d mat: %d\n",
                 (fKey >> kDepthShift) & kDepthMask,
                 (fKey >> kMaterialShift) & kMaterialMask);
    }

    bool operator>(const Key& other) const { return fKey > other.fKey; }
    bool operator<(const Key& other) const { return fKey < other.fKey; }

private:
    uint64_t fKey;
};

class KeyAndCmd {
public:
    Key  fKey;
    Cmd* fCmd;
};

class FakePaint {
public:
    FakePaint() {}

    void setColor(SkColor c) { fColor = c; }
    SkColor getColor() const { return fColor; }

protected:

private:
    SkColor fColor = SK_ColorBLACK;
};

class FakeDevice {
public:
    FakeDevice(SkBitmap bm) : fBM(bm) {
        SkASSERT(bm.width() == 256 && bm.height() == 256);

        memset(fZBuffer, 0, sizeof(fZBuffer));
    }

    ~FakeDevice() {
    }

    void drawRect(int id, uint32_t z, SkIRect r, FakePaint p);

    void sort() {
        std::sort(fSortedCmds.begin(), fSortedCmds.end(),
                  [](const KeyAndCmd& a, const KeyAndCmd& b) {
                      return a.fKey < b.fKey;
                  });
    }

    void finalize();

    void getOrder(std::vector<int>*) const;

protected:

private:
    bool                   fFinalized = false;
    std::vector<KeyAndCmd> fSortedCmds;

    SkBitmap               fBM;
    uint32_t               fZBuffer[256][256];
};

class FakeCanvas {
public:
    FakeCanvas(SkBitmap& bm) {
        fStateStack.push_back({bm, SkMatrix::I()});
    }

    void saveLayer() {
        SkASSERT(!fFinalized);

    }

    void drawRect(int id, SkIRect r, FakePaint p);

    void clipRect(SkIRect r) {
        SkASSERT(!fFinalized);

    }

    void translate(SkVector trans) {
        SkASSERT(!fFinalized);

    }

    void finalize();

    std::vector<int> getOrder() const;

protected:

private:
    uint32_t nextZ() {
        return fNextZ++;
    }

    class State {
    public:
        FakeDevice fDevice;
        SkMatrix   fCTM;
    };

    int                    fNextZ = 1;
    bool                   fFinalized = false;
    std::vector<State>     fStateStack;
};


#endif // Fake_DEFINED
