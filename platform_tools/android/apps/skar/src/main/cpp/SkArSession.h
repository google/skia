/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKAR_SKARSESSION
#define SKAR_SKARSESSION

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <errno.h>
#include <jni.h>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <SkMatrix44.h>
#include <SkRefCnt.h>

#include "arcore_c_api.h"
#include "glm.h"
#include "SkArUtil.h"

namespace SkAR {
    class SkArSession : public SkRefCnt {
    public:
        static sk_sp<SkArSession> Make(void* env, void* context, void* activity);

        ~SkArSession();

        const ArSession* getArSession() const;

        ArSession* getArSession();
    private:
        typedef SkRefCnt INHERITED;
        SkArSession(void* env, void* context, void* activity);

        // ArSession is managed by SkArSession
        ArSession* fArSession;
    };

}  // namespace SkAr

#endif  // SKAR_SKARSESSION
