/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottyShapes_DEFINED
#define SkottyShapes_DEFINED

#include "SkString.h"
#include "SkTypes.h"

#include <memory>

// testing
class SkCanvas;

namespace Json { class Value; }

namespace sksg { class RenderNode;  }

namespace skotty {

class ShapeBase : public SkNoncopyable {
public:
    virtual ~ShapeBase() = default;

    static std::unique_ptr<ShapeBase> Make(const Json::Value&);

    sk_sp<sksg::RenderNode> attach() const {
        return this->onAttach();
    }

    // testing
    virtual void render(SkCanvas*) const {}

protected:
    ShapeBase(const Json::Value& shape);

    virtual sk_sp<sksg::RenderNode> onAttach() const = 0;

private:
    SkString fName,
             fMatchName;

    typedef SkNoncopyable INHERITED;
private:
};

} // namespace

#endif // SkottyShapes_DEFINED
