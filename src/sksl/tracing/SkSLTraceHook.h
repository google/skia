/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSLTRACEHOOK
#define SKSLTRACEHOOK

#include <cstdint>
#include <memory>
#include <vector>

namespace SkSL {

struct TraceInfo;

class TraceHook {
public:
    virtual ~TraceHook() = default;
    virtual void line(int lineNum) = 0;
    virtual void var(int slot, int32_t val) = 0;
    virtual void enter(int fnIdx) = 0;
    virtual void exit(int fnIdx) = 0;
    virtual void scope(int delta) = 0;
};

class Tracer : public TraceHook {
public:
    static std::unique_ptr<Tracer> Make(std::vector<TraceInfo>* traceInfo);

    void line(int lineNum) override;
    void var(int slot, int32_t val) override;
    void enter(int fnIdx) override;
    void exit(int fnIdx) override;
    void scope(int delta) override;

private:
    std::vector<TraceInfo>* fTraceInfo;
};

}  // namespace SkSL

#endif
