/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GpuTimer_DEFINED
#define GpuTimer_DEFINED

class SkGLContext;

class GpuTimer {
public:
    GpuTimer(const SkGLContext*);
    ~GpuTimer();
    void start();
    double end();
private:
    unsigned fQuery;
    int fStarted;
    const SkGLContext* fContext;
    bool fSupported;
};

#endif
