/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGNode_DEFINED
#define SkSGNode_DEFINED

#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"

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
        kBubbleDamage_Trait = 1 << 0,
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
        Node*             fInvalObserver;
        SkTDArray<Node*>* fInvalObserverArray;
    };
    SkRect                fBounds;
    const uint32_t        fInvalTraits : 16;
    uint32_t              fFlags       : 16;

    typedef SkRefCnt INHERITED;
};

// Helper for defining attribute getters/setters in subclasses.
#define SG_ATTRIBUTE(attr_name, attr_type, attr_container)             \
    const attr_type& get##attr_name() const { return attr_container; } \
    void set##attr_name(const attr_type& v) {                          \
        if (attr_container == v) return;                               \
        attr_container = v;                                            \
        this->invalidate();                                            \
   }

} // namespace sksg

#endif // SkSGNode_DEFINED
