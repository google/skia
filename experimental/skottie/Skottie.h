/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Skottie_DEFINED
#define Skottie_DEFINED

#include "SkRefCnt.h"
#include "SkSize.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTHash.h"
#include "SkTypes.h"

#include <memory>

class SkCanvas;
struct SkRect;
class SkStream;

namespace Json { class Value; }

namespace sksg { class Scene;  }

namespace skottie {

class ResourceProvider : public SkNoncopyable {
public:
    virtual ~ResourceProvider() = default;

    virtual std::unique_ptr<SkStream> openStream(const char resource[]) const = 0;
};

class Animation : public SkRefCnt {
public:
    static sk_sp<Animation> Make(SkStream*, const ResourceProvider&);
    static sk_sp<Animation> MakeFromFile(const char path[], const ResourceProvider* = nullptr);

    ~Animation() override;

    void render(SkCanvas*, const SkRect* dst = nullptr) const;

    void animationTick(SkMSec);

    const SkString& version() const { return fVersion;   }
    const SkSize&      size() const { return fSize;      }
         SkScalar frameRate() const { return fFrameRate; }
         SkScalar   inPoint() const { return fInPoint;   }
         SkScalar  outPoint() const { return fOutPoint;  }

    void setShowInval(bool show);

private:
    Animation(const ResourceProvider&,
              SkString ver, const SkSize& size, SkScalar fps,
              const Json::Value&);

    SkString                     fVersion;
    SkSize                       fSize;
    SkScalar                     fFrameRate,
                                 fInPoint,
                                 fOutPoint;

    std::unique_ptr<sksg::Scene> fScene;

    typedef SkRefCnt INHERITED;
};

} // namespace skottie

#endif // Skottie_DEFINED
