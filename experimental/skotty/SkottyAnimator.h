/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottyAnimator_DEFINED
#define SkottyAnimator_DEFINED

#include "SkottyProperties.h"
#include "SkTypes.h"

#include <functional>
#include <memory>

namespace skotty {

class AnimatorBase : public SkNoncopyable {
public:
    virtual ~AnimatorBase() = default;

    virtual void tick(SkMSec) = 0;

protected:
    AnimatorBase()  = default;
};

template <typename PropT, typename AttrT, typename NodeT>
class Animator : public AnimatorBase {
public:
    Animator(sk_sp<NodeT> node, std::unique_ptr<Keyframed<PropT>> prop,
             std::function<void(const sk_sp<NodeT>&, const AttrT&)> applyFunc)
        : fProp(std::move(prop))
        , fTarget(std::move(node))
        , fFunc(std::move(applyFunc)) {}

    void tick(SkMSec t) override {
        PropT val;
        fProp->eval(t, &val);
        fFunc(fTarget, val.template as<AttrT>());
    }

private:
    std::unique_ptr<Keyframed<PropT>>                      fProp;
    sk_sp<NodeT>                                           fTarget;
    std::function<void(const sk_sp<NodeT>&, const AttrT&)> fFunc;
};

} // namespace skotty

#endif // SkottyAnimator_DEFINED
