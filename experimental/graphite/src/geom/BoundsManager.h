/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_geom_BoundsManager_DEFINED
#define skgpu_geom_BoundsManager_DEFINED

#include <cstdint>

struct SkIRect;

namespace skgpu {

/**
 * BoundsManager is an acceleration structure for device-space related pixel bounds queries.
 * It supports querying the maximum previously written Z value within a given bounds, recording a
 * new Z value within a bounding rect, and querying if a {bounds, Z} tuple would be fully occluded
 * by a later operation.
 */
class BoundsManager {
public:
    virtual ~BoundsManager() {}

    virtual uint16_t getMaxZ(const SkIRect& bounds) const = 0;

    virtual bool isOccluded(const SkIRect& bounds, uint16_t z) const = 0;

    virtual void setZ(const SkIRect& bounds, uint16_t z, bool fullyOpaque=false) = 0;
};

// TODO: Select one most-effective BoundsManager implementation, make it the only option, and remove
// virtual-ness. For now, this seems useful for correctness testing by comparing against trivial
// implementations and for identifying how much "smarts" are actually worthwhile.

// A BoundsManager that produces exact painter's order and assumes nothing is occluded.
class NaiveBoundsManager final : public BoundsManager {
public:
    ~NaiveBoundsManager() override {}

    uint16_t getMaxZ(const SkIRect& bounds) const override { return fMaxZ; }

    bool isOccluded(const SkIRect& bounds, uint16_t z) const override { return false; }

    void setZ(const SkIRect& bounds, uint16_t z, bool fullyOpaque=false) override {
        if (z > fMaxZ) {
            fMaxZ = z;
        }
    }

private:
    uint16_t fMaxZ = 0;
};

// A BoundsManager that tracks every {bounds, Z} tuple and can exactly determine all queries
// using a brute force search.
class BruteForceBoundsManager : public BoundsManager {
public:
    ~BruteForceBoundsManager() override {}

    // TODO: implement this class
    uint16_t getMaxZ(const SkIRect& bounds) const override { return 0; }

    bool isOccluded(const SkIRect& bounds, uint16_t z) const override { return false; }

    void setZ(const SkIRect& bounds, uint16_t z, bool fullyOpaque=false) override {}
};

} // namespace skgpu

#endif // skgpu_geom_BoundsManager_DEFINED
