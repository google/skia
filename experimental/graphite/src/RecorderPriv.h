/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RecorderPriv_DEFINED
#define skgpu_RecorderPriv_DEFINED

#include "experimental/graphite/include/Recorder.h"

namespace skgpu {

class RecorderPriv {
public:
    void add(sk_sp<Task>);

    ResourceProvider* resourceProvider() const;
    UniformCache* uniformCache() const;
    DrawBufferManager* drawBufferManager() const;
    const Caps* caps() const;

private:
    explicit RecorderPriv(Recorder* recorder) : fRecorder(recorder) {}
    RecorderPriv& operator=(const RecorderPriv&) = delete;

    // No taking addresses of this type.
    const RecorderPriv* operator&() const = delete;
    RecorderPriv* operator&() = delete;

    Recorder* fRecorder;

    friend class Recorder;  // to construct/copy this type.

};

inline RecorderPriv Recorder::priv() {
    return RecorderPriv(this);
}

inline const RecorderPriv Recorder::priv() const {  // NOLINT(readability-const-return-type)
    return RecorderPriv(const_cast<Recorder*>(this));
}

} // namespace skgpu

#endif // skgpu_RecorderPriv_DEFINED
