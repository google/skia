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
    
    SkThread(entryPointProc entryPoint, void* data = NULL);
    
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
    
    /**
     * SkThreads with an affinity for the same processor will attempt to run cache
     * locally with each other. SkThreads with an affinity for different processors
     * will attempt to run on different cores. Returns false if the request failed.
     */
    bool setProcessorAffinity(unsigned int processor);
    
private:
    void* fData;
};

#endif
