/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Recorder_DEFINED
#define skgpu_Recorder_DEFINED

#include "experimental/graphite/src/TaskGraph.h"
#include "include/core/SkRefCnt.h"

namespace skgpu {

class Context;
class DrawBufferManager;
class Recording;
class UniformCache;

class Recorder final : public SkRefCnt {
public:
    ~Recorder() override;

    void add(sk_sp<Task>);

    Context* context() const;
    UniformCache* uniformCache();
    DrawBufferManager* drawBufferManager();

    std::unique_ptr<Recording> snap();

protected:
private:
    friend class Context; // For ctor
    Recorder(sk_sp<Context>);

    sk_sp<Context> fContext;
    TaskGraph fGraph;
    std::unique_ptr<UniformCache> fUniformCache;
    std::unique_ptr<DrawBufferManager> fDrawBufferManager;
};

} // namespace skgpu

#endif // skgpu_Recorder_DEFINED
