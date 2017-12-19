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

class Node : public SkRefCnt {
public:
    void revalidate(InvalidationController*, const SkMatrix&);

protected:
    Node();
    ~Node() override;

    void invalidate();
    bool isInvalidated() const { return fFlags & kInvalidated_Flag; }
    virtual void onRevalidate(InvalidationController*, const SkMatrix&) = 0;

private:
    void addParent(Node*);
    void removeParent(Node*);
    friend class ContainerNode;
    friend class Draw;
    friend class EffectNode;

    template <typename Func>
    void forEachParent(Func&&);

    enum Flags {
        kInvalidated_Flag  = 1 << 0,
        kMultiParents_Flag = 1 << 1,
        kInTraversal_Flag  = 1 << 2,
    };

    class ScopedFlag;

    union {
        Node*             fParent;
        SkTDArray<Node*>* fParentArray;
    };
    uint32_t              fFlags;

    typedef SkRefCnt INHERITED;
};

#define MAPPED_ATTRIBUTE(attr_name, attr_type, attr_container) \
    attr_type attr_name() const { return attr_container; }     \
    void set_##attr_name(attr_type v) {                        \
        if (attr_container == v) return;                       \
        attr_container = v;                                    \
        this->invalidate();                                    \
   }

} // namespace sksg

#endif // SkSGNode_DEFINED
