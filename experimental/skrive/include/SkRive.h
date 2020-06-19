/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRive_DEFINED
#define SkRive_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"

#include <memory>
#include <type_traits>
#include <vector>

class SkCanvas;
class SkStreamAsset;

namespace skrive {

#define ACTOR_ATTR(attr_name, attr_type, attr_default)               \
private:                                                             \
    attr_type f##attr_name = attr_default;                           \
public:                                                              \
    const attr_type& get##attr_name() const { return f##attr_name; } \
    void set##attr_name(const attr_type& v) {                        \
        if (f##attr_name == v) return;                               \
        f##attr_name = v;                                            \
        this->invalidate();                                          \
    }                                                                \
    void set##attr_name(attr_type&& v) {                             \
        if (f##attr_name == v) return;                               \
        f##attr_name = std::move(v);                                 \
        this->invalidate();                                          \
    }

class Node;

class Component : public SkRefCnt {
public:
    ACTOR_ATTR(Name, SkString, SkString())

    template <typename T>
    std::enable_if_t<std::is_base_of<Component, T>::value, bool>
    is() const {
        if constexpr(std::is_same<Component, T>::value) {
            return true;
        } else {
            return is_base_of<T>(fType);
        }
    }

    template <typename T>
    operator const T*() const {
        return this->is<T>() ? reinterpret_cast<const T*>(this) : nullptr;
    }

    template <typename T>
    operator T*() {
        return this->is<T>() ? reinterpret_cast<T*>(this) : nullptr;
    }

    void revalidate();

protected:
    enum class Type : uint32_t {
        kNode,
        kShape,
    };

    explicit Component(Type t) : fType(t) {}

    void invalidate();

    bool hasInval() const { return fDirty; }

    virtual void onRevalidate() = 0;

private:
    friend class Node; // parent access

    template <typename T>
    static constexpr bool is_base_of(Type t);

    const Type  fType;

    Node* fParent = nullptr;
    bool  fDirty  = true;
};

class Node : public Component {
public:
    Node() : INHERITED(Type::kNode) {}

    ACTOR_ATTR(Translation        , SkV2 , SkV2({0, 0}))
    ACTOR_ATTR(Scale              , SkV2 , SkV2({1, 1}))
    ACTOR_ATTR(Rotation           , float, 0           )
    ACTOR_ATTR(Opacity            , float, 1           )
    ACTOR_ATTR(CollapsedVisibility, bool , false       )

    void addChild(sk_sp<Component>);

protected:
    explicit Node(Type t) : INHERITED(t) {}

private:
    void onRevalidate() override;

    std::vector<sk_sp<Component>> fChildren;

    using INHERITED = Component;
};

class Drawable : public Node {
public:
    ACTOR_ATTR(DrawOrder, size_t     , 0                    )
    ACTOR_ATTR(BlendMode, SkBlendMode, SkBlendMode::kSrcOver)
    ACTOR_ATTR(IsHidden , bool       , false                )

protected:
    explicit Drawable(Type t) : INHERITED(t) {}

private:
    using INHERITED = Node;
};

class Shape final : public Drawable {
public:
    Shape() : INHERITED(Type::kShape) {}

    ACTOR_ATTR(TransformAffectsStroke, bool, true)

private:
    void onRevalidate() override;

    using INHERITED = Drawable;
};

template <typename T>
constexpr bool Component::is_base_of(Type t) {
    if (t == Type::kNode ) return std::is_base_of<T, Node >::value;
    if (t == Type::kShape) return std::is_base_of<T, Shape>::value;

    return false;
}

class Artboard final : public SkRefCnt {
public:
    ACTOR_ATTR(Name        , SkString , SkString()      )
    ACTOR_ATTR(Color       , SkColor4f, SkColors::kBlack)
    ACTOR_ATTR(Size        , SkV2     , SkV2({0,0})     )
    ACTOR_ATTR(Origin      , SkV2     , SkV2({0,0})     )
    ACTOR_ATTR(Translation , SkV2     , SkV2({0,0})     )
    ACTOR_ATTR(ClipContents, bool     , false           )

    void render(SkCanvas*) const;

private:
    void invalidate() {}
};

class SK_API SkRive final : public SkNVRefCnt<SkRive> {
public:
    class Builder final {
    public:
        sk_sp<SkRive> make(std::unique_ptr<SkStreamAsset>);
    };

    const std::vector<sk_sp<Artboard>>& artboards() const { return fArtboards; }
          std::vector<sk_sp<Artboard>>& artboards()       { return fArtboards; }

private:
    std::vector<sk_sp<Artboard>> fArtboards;
};

} // skrive

#endif // SkRive_DEFINED
