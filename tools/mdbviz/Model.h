/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/private/SkTDArray.h"

class DrawCommand;

// This class encapsulates the both the in-memory representation of the draw ops
// and the state of Skia/Ganesh's rendering. It should never have any Qt intrusions.
class Model {
public:
    enum class ErrorCode {
        kOK,
        kCouldntOpenFile,
        kCouldntDecodeSKP
    };

    Model();
    ~Model();

    static const char* ErrorString(ErrorCode);

    // Replace the list of draw ops by reading the provided skp filename and
    // reset the Skia draw state. It is up to the view portion to update itself
    // after this call (i.e., rebuild the opsTask view).
    ErrorCode load(const char* filename);

    // Update the rendering state to the provided op
    void setCurOp(int curOp);
    int curOp() const { return fCurOp; }

    int numOps() const { return fOps.count(); }
    const char* getOpName(int index) const;

    bool isHierarchyPush(int index) const;
    bool isHierarchyPop(int index) const;

    // Get the bits visually representing the current rendering state
    void* getPixels() const { return fBM.getPixels(); }
    int width() const { return fBM.width(); }
    int height() const { return fBM.height(); }

protected:
    // draw the ops up to (and including) the index-th op
    void drawTo(int index);
    void resetOpsTask();

private:
    SkTDArray<DrawCommand*>   fOps;
    int                       fCurOp;  // The current op the rendering state is at
    SkBitmap                  fBM;
};
