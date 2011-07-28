
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBenchGpuTimer_DEFINED
#define SkBenchGpuTimer_DEFINED

class BenchGpuTimer {
public:
    BenchGpuTimer();
    ~BenchGpuTimer();
    void startGpu();
    double endGpu();
};

#endif
