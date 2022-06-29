/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RecorderPriv_DEFINED
#define skgpu_graphite_RecorderPriv_DEFINED

#include "include/gpu/graphite/Recording.h"

namespace skgpu::graphite {

class RecordingPriv {
public:
    bool addCommands(ResourceProvider*, CommandBuffer*);
    void addResourceRef(sk_sp<Resource> resource);

private:
    explicit RecordingPriv(Recording* recorder) : fRecording(recorder) {}
    RecordingPriv& operator=(const RecordingPriv&) = delete;

    // No taking addresses of this type.
    const RecordingPriv* operator&() const = delete;
    RecordingPriv* operator&() = delete;

    Recording* fRecording;

    friend class Recording;  // to construct/copy this type.
};

inline RecordingPriv Recording::priv() {
    return RecordingPriv(this);
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_RecordingPriv_DEFINED

