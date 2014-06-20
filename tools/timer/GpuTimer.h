/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GpuTimer_DEFINED
#define GpuTimer_DEFINED

class SkGLContextHelper;

class GpuTimer {
public:
    GpuTimer(const SkGLContextHelper*);
    ~GpuTimer();
    void start();
    double end();
private:
    unsigned fQuery;
    int fStarted;
    const SkGLContextHelper* fContext;
    bool fSupported;
};

#endif
