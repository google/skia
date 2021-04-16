// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Fake_DEFINED
#define Fake_DEFINED

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

    void dump() const {
        SkDebugf("depth: %d mat %d\n",
                 (fKey >> kDepthShift) & kDepthMask,
                 (fKey >> kMaterialShift) & kMaterialMask);
    }

    bool operator>(const Key& other) const {
        return fKey > other.fKey;
    }

private:
    uint64_t fKey;
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

protected:

private:
};

class FakeCanvas {
public:
    FakeCanvas() {
        fStateStack.push_back(State());
    }

    void saveLayer() {
        SkASSERT(!fFinalized);

    }

    void drawRect(int id, SkRect r, FakePaint p);

    void clipRect(SkRect r) {
        SkASSERT(!fFinalized);

    }

    void translate(SkVector trans) {
        SkASSERT(!fFinalized);

    }

    void finalize() {
        SkASSERT(!fFinalized);
        fFinalized = true;

        std::sort(fSortedCmds.begin(), fSortedCmds.end(),
                  [](const KeyAndCmd& a, const KeyAndCmd& b) {
                      return a.fKey > b.fKey;
                  });
    }

    std::vector<int> getOrder() const;

    void replay(SkBitmap* dstBM) const;

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

    class KeyAndCmd {
    public:
        Key  fKey;
        Cmd* fCmd;
    };

    int                    fNextZ = 1;
    bool                   fFinalized = false;
    std::vector<State>     fStateStack;
    std::vector<KeyAndCmd> fSortedCmds;
};


#endif // Fake_DEFINED