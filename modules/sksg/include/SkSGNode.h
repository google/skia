/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGNode_DEFINED
#define SkSGNode_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"

#include <vector>

class SkCanvas;
class SkMatrix;

namespace sksg {

class InvalidationController;

/**
 * Base class for all scene graph nodes.
 *
 * Handles ingress edge management for the DAG (i.e. node -> "parent" node mapping),
 * and invalidation.
 *
 * Note: egress edges are only implemented/supported in container subclasses
 * (e.g. Group, Effect, Draw).
 */
class Node : public SkRefCnt {
public:
    // Traverse the DAG and revalidate any dependant/invalidated nodes.
    // Returns the bounding box for the DAG fragment.
    const SkRect& revalidate(InvalidationController*, const SkMatrix&);

protected:
    enum InvalTraits {
        // Nodes with this trait never generate direct damage -- instead,
        // the damage bubbles up to ancestors.
        kBubbleDamage_Trait   = 1 << 0,

        // Nodes with this trait obscure the descendants' damage and always override it.
        kOverrideDamage_Trait = 1 << 1,
    };

    explicit Node(uint32_t invalTraits);
    ~Node() override;

    const SkRect& bounds() const {
        SkASSERT(!this->hasInval());
        return fBounds;
    }

    // Tag this node for invalidation and optional damage.
    void invalidate(bool damage = true);
    bool hasInval() const { return fFlags & kInvalidated_Flag; }

    // Dispatched on revalidation.  Subclasses are expected to recompute/cache their properties
    // and return their bounding box in local coordinates.
    virtual SkRect onRevalidate(InvalidationController*, const SkMatrix& ctm) = 0;

    // Register/unregister |this| to receive invalidation events from a descendant.
    void observeInval(const sk_sp<Node>&);
    void unobserveInval(const sk_sp<Node>&);

private:
    enum Flags {
        kInvalidated_Flag   = 1 << 0, // the node or its descendants require revalidation
        kDamage_Flag        = 1 << 1, // the node contributes damage during revalidation
        kObserverArray_Flag = 1 << 2, // the node has more than one inval observer
        kInTraversal_Flag   = 1 << 3, // the node is part of a traversal (cycle detection)
    };

    template <typename Func>
    void forEachInvalObserver(Func&&) const;

    class ScopedFlag;

    union {
        Node*               fInvalObserver;
        std::vector<Node*>* fInvalObserverArray;
    };
    SkRect                  fBounds;
    const uint32_t          fInvalTraits :  2;
    uint32_t                fFlags       :  4; // Internal flags.
    uint32_t                fNodeFlags   :  8; // Accessible from select subclasses.
    // Free bits                         : 18;

    friend class RenderNode; // node flags access

    typedef SkRefCnt INHERITED;
};

// Helper for defining attribute getters/setters in subclasses.
#define SG_ATTRIBUTE(attr_name, attr_type, attr_container)             \
    const attr_type& get##attr_name() const { return attr_container; } \
    void set##attr_name(const attr_type& v) {                          \
        if (attr_container == v) return;                               \
        attr_container = v;                                            \
        this->invalidate();                                            \
    }                                                                  \
    void set##attr_name(attr_type&& v) {                               \
        if (attr_container == v) return;                               \
        attr_container = std::move(v);                                 \
        this->invalidate();                                            \
    }

#define SG_MAPPED_ATTRIBUTE(attr_name, attr_type, attr_container)                \
    attr_type get##attr_name() const { return attr_container.get##attr_name(); } \
    void set##attr_name(const attr_type& v) {                                    \
        if (attr_container.get##attr_name() == v) return;                        \
        attr_container.set##attr_name(v);                                        \
        this->invalidate();                                                      \
    }                                                                            \
    void set##attr_name(attr_type&& v) {                                         \
        if (attr_container.get##attr_name() == v) return;                        \
        attr_container.set##attr_name(std::move(v));                             \
        this->invalidate();                                                      \
    }

} // namespace sksg

#endif // SkSGNode_DEFINED
