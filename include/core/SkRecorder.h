/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRecorder_DEFINED
#define SkRecorder_DEFINED

#include "include/private/base/SkAPI.h"

class SkCanvas;

namespace skcpu {
class Recorder;
}

class SK_API SkRecorder {
public:
    SkRecorder() = default;
    virtual ~SkRecorder() = default;
    SkRecorder(const SkRecorder&) = delete;
    SkRecorder(SkRecorder&&) = delete;
    SkRecorder& operator=(const SkRecorder&) = delete;

    enum class Type {
        kCPU,
        kGanesh,
        kGraphite,
    };

    virtual Type type() const = 0;

    virtual skcpu::Recorder* cpuRecorder() = 0;

private:

    /**
    * Attempts to create and return an SkCaptureCanvas that wraps the provided base canvas.
    * Returns nullptr if capture is not enabled.
    */
    virtual SkCanvas* makeCaptureCanvas(SkCanvas*) = 0;

    friend class SkSurface_Base;
};

#endif
