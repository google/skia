/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrScissorState_DEFINED
#define GrScissorState_DEFINED

class GrScissorState {
public:
    GrScissorState() = default;
    GrScissorState(const SkIRect& rect) : fScissorTest(GrScissorTest::kEnabled), fRect(rect) {}
    void setDisabled() { fScissorTest = GrScissorTest::kDisabled; }
    bool SK_WARN_UNUSED_RESULT intersect(const SkIRect& rect) {
        if (fScissorTest == GrScissorTest::kDisabled) {
            *this = GrScissorState(rect);
            return true;
        }
        return fRect.intersect(rect);
    }
    bool operator==(const GrScissorState& other) const {
        return fScissorTest == other.fScissorTest &&
               (fScissorTest == GrScissorTest::kDisabled || fRect == other.fRect);
    }
    bool operator!=(const GrScissorState& other) const { return !(*this == other); }

    GrScissorTest scissorTest() const { return fScissorTest; }
    const SkIRect& rect() const { return fRect; }

private:
    GrScissorTest fScissorTest = GrScissorTest::kDisabled;
    SkIRect fRect;
};

#endif
