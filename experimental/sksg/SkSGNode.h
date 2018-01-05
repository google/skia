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
    Node();
    ~Node() override;

    void invalidateSelf();
    void invalidateAncestors();

    bool hasSelfInval()       const { return fFlags & kInvalSelf_Flag; }
    bool hasDescendantInval() const { return fFlags & kInvalDescendant_Flag; }
    bool hasInval()           const { return this->hasSelfInval() || this->hasDescendantInval(); }

    // Dispatched on revalidation.  Subclasses are expected to recompute/cache their properties
    // and return their bounding box in local coordinates.
    enum class Damage {
        kDefault,    // respects the local kInvalSelf_Flag
        kForceSelf,  // forces self revalidation regardless of kInvalSelf_Flag
        kBlockSelf,  // blocks self revalidation regardless of kInvalSelf_Flag
    };
    struct RevalidationResult {
        SkRect  fBounds;
        Damage  fDamage;
    };
    virtual RevalidationResult onRevalidate(InvalidationController*, const SkMatrix& ctm) = 0;

private:
    void addInvalReceiver(Node*);
    void removeInvalReceiver(Node*);
    // TODO: too friendly, find another way.
    friend class Draw;
    friend class EffectNode;
    friend class Group;
    friend class Matrix;
    friend class Merge;
    friend class Stroke;
    friend class Transform;

    template <typename Func>
    void forEachInvalReceiver(Func&&) const;

    enum Flags {
        kInvalSelf_Flag       = 1 << 0, // the node requires revalidation
        kInvalDescendant_Flag = 1 << 1, // the node's descendents require invalidation
        kReceiverArray_Flag   = 1 << 2, // the node has more than one inval receiver
        kInTraversal_Flag     = 1 << 3, // the node is part of a traversal (cycle detection)
    };

    class ScopedFlag;

    union {
        Node*             fInvalReceiver;
        SkTDArray<Node*>* fInvalReceiverArray;
    };
    SkRect                fBounds;
    uint32_t              fFlags;

    typedef SkRefCnt INHERITED;
};

// Helper for defining attribute getters/setters in subclasses.
#define SG_ATTRIBUTE(attr_name, attr_type, attr_container)      \
    attr_type get##attr_name() const { return attr_container; } \
    void set##attr_name(attr_type v) {                          \
        if (attr_container == v) return;                        \
        attr_container = v;                                     \
        this->invalidateSelf();                                 \
   }

} // namespace sksg

#endif // SkSGNode_DEFINED
