/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGNode_DEFINED
#define SkSGNode_DEFINED

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
    // Traverse the DAG and revalidate any connected/invalidated nodes.
    void revalidate(InvalidationController*, const SkMatrix&);

protected:
    Node();
    ~Node() override;

    // Mark this node and (transitively) any invalidation receivers for revalidation.
    void invalidate();

    bool isInvalidated() const { return fFlags & kInvalidated_Flag; }

    // Dispatched on revalidation.  Subclasses are expected to recompute their geometry
    // and push dirty rects to the InvalidationController.
    virtual void onRevalidate(InvalidationController*, const SkMatrix& ctm) = 0;

private:
    void addInvalReceiver(Node*);
    void removeInvalReceiver(Node*);
    friend class Draw;
    friend class EffectNode;
    friend class Group;
    friend class Stroke;

    template <typename Func>
    void forEachInvalReceiver(Func&&) const;

    enum Flags {
        kInvalidated_Flag   = 1 << 0, // the node requires revalidation
        kReceiverArray_Flag = 1 << 1, // the node has more than one inval receiver
        kInTraversal_Flag   = 1 << 2, // the node is part of a traversal (cycle detection)
    };

    class ScopedFlag;

    union {
        Node*             fInvalReceiver;
        SkTDArray<Node*>* fInvalReceiverArray;
    };
    uint32_t              fFlags;

    typedef SkRefCnt INHERITED;
};

// Helper for defining attribute getters/setters in subclasses.
#define SG_ATTRIBUTE(attr_name, attr_type, attr_container) \
    attr_type get##attr_name() const { return attr_container; } \
    void set##attr_name(attr_type v) {                    \
        if (attr_container == v) return;                   \
        attr_container = v;                                \
        this->invalidate();                                \
   }

} // namespace sksg

#endif // SkSGNode_DEFINED
