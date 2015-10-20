/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkThreadUtils_DEFINED
#define SkThreadUtils_DEFINED

#include "SkTypes.h"

class SkThread : SkNoncopyable {
public:
    typedef void (*entryPointProc)(void*);

    SkThread(entryPointProc entryPoint, void* data = nullptr);

    /**
     * Non-virtual, do not subclass.
     */
    ~SkThread();

    /**
     * Starts the thread. Returns false if the thread could not be started.
     */
    bool start();

    /**
     * Waits for the thread to finish.
     * If the thread has not started, returns immediately.
     */
    void join();

private:
    void* fData;
};

#endif
