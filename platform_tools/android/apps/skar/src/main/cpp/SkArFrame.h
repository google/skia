/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArFrame_DEFINED
#define SkArFrame_DEFINED

#include <SkRefCnt.h>
#include <arcore_c_api.h>
#include "SkArSession.h"

class SkArFrame : public SkRefCnt{
    /*
     * Describes the world state after calling an update() using an SkArSession.
     * Create this once using the factory method, then update it using SkArSession each drawing
     * frame.
     */

public:
    /**
     * Factory method used to construct an SkArFrame once.
     * @param arSession shared pointer to the current SkArSession
     * @return          shared pointer to the current SkArFrame
     */
    static sk_sp<SkArFrame> Make(sk_sp<SkArSession> arSession);

    ~SkArFrame();

    const ArFrame* getArFrame() const;

    ArFrame* getArFrame();

private:
    SkArFrame(sk_sp<SkArSession> arSession);

    // ArSession is managed by SkArSession
    ArFrame* fArFrame;

    typedef SkRefCnt INHERITED;
};

#endif  // SkArFrame_DEFINED


