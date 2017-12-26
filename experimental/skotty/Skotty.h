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
#include "SkTHash.h"
#include "SkTypes.h"

#include <memory>

class SkCanvas;
class SkStream;

namespace Json { class Value; }

namespace sksg { class RenderNode;  }

namespace skotty {

class Animation : public SkNoncopyable {
public:
    static std::unique_ptr<Animation> Make(SkStream*);

    ~Animation();

    void render(SkCanvas*) const;

    void animationTick(SkMSec);

    const SkString& version() const { return fVersion; }
    const   SkSize&    size() const { return    fSize; }

private:
    class Composition;

    class Layer;
    class CompLayer;
    class ImageLayer;
    class NullLayer;
    class TextLayer;
    class ShapeLayer;
    class SolidLayer;

    Animation(SkString ver, const SkSize& size,
              std::unique_ptr<Composition> root,
              const Json::Value&);

    SkString                                           fVersion;
    SkSize                                             fSize;
    std::unique_ptr<Composition>                       fRoot;

    // assets
    SkTHashMap<SkString, std::unique_ptr<Composition>> fCompositions;

    // Attached DOM
    sk_sp<sksg::RenderNode>                            fDom;

    typedef SkNoncopyable INHERITED;
};

} // namespace skotty

#endif // Skotty_DEFINED
