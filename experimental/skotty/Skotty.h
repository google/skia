/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Skotty_DEFINED
#define Skotty_DEFINED

#include "SkRefCnt.h"
#include "SkSize.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTHash.h"
#include "SkTypes.h"

#include <memory>

class SkCanvas;
class SkStream;

namespace Json { class Value; }

namespace sksg { class RenderNode;  }

namespace skotty {

class AnimatorBase;

class Animation : public SkNoncopyable {
public:
    static std::unique_ptr<Animation> Make(SkStream*);

    ~Animation();

    void render(SkCanvas*) const;

    void animationTick(SkMSec);

    const SkString& version() const { return fVersion;   }
    const SkSize&      size() const { return fSize;      }
         SkScalar frameRate() const { return fFrameRate; }
         SkScalar   inPoint() const { return fInPoint;   }
         SkScalar  outPoint() const { return fOutPoint;  }

    void setShowInval(bool show) { fShowInval = show; }

private:
    Animation(SkString ver, const SkSize& size, SkScalar fps, const Json::Value&);

    SkString                                fVersion;
    SkSize                                  fSize;
    SkScalar                                fFrameRate,
                                            fInPoint,
                                            fOutPoint;

    sk_sp<sksg::RenderNode>                 fDom;
    SkTArray<std::unique_ptr<AnimatorBase>> fAnimators;

    bool                    fShowInval = false;

    typedef SkNoncopyable INHERITED;
};

} // namespace skotty

#endif // Skotty_DEFINED
