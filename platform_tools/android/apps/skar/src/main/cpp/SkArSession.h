/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKAR_SKARSESSION
#define SKAR_SKARSESSION


#include <SkRefCnt.h>
#include <arcore_c_api.h>

class SkArFrame;

class SkArSession : public SkRefCnt {
public:
    static sk_sp<SkArSession> Make(void* env, void* context);

    ~SkArSession();

    bool pause();

    bool resume();

    bool update(sk_sp<SkArFrame> arFrame);

    void setDisplayGeometry(int32_t rotation, int32_t width, int32_t height);

    void setCameraTextureName(uint32_t textureId);

    const ArSession* getArSession() const;

    ArSession* getArSession();
private:
    typedef SkRefCnt INHERITED;
    SkArSession(void* env, void* context);

    // ArSession is managed by SkArSession
    ArSession* fArSession;
};

#endif  // SKAR_SKARSESSION


#ifndef SKAR_SKARFRAME
#define SKAR_SKARFRAME

#include <SkRefCnt.h>
#include <arcore_c_api.h>
#include "SkArSession.h"

class SkArFrame : public SkRefCnt{
public:
    static sk_sp<SkArFrame> Make(sk_sp<SkArSession> arSession);

    ~SkArFrame();

    const ArFrame* getArFrame() const;
    ArFrame* getArFrame();

private:
    typedef SkRefCnt INHERITED;
    SkArFrame(sk_sp<SkArSession> arSession);

    // ArSession is managed by SkArSession
    ArFrame* fArFrame;
};
#endif  // SKAR_SKARFRAME
