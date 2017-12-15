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

namespace sksg {

class Node : public SkRefCnt {
public:

protected:
    Node();
    ~Node() override;

    enum {
        kPaint_Inval    = 1 << 0,
        kGeometry_Inval = 1 << 1,

        kFull_Inval     = kPaint_Inval | kGeometry_Inval,
    };
    void invalidate(uint32_t invalMask);

    uint32_t getInvalState() const { return fInvalState; }
    void clearInvalState(uint32_t invalMask) { fInvalState &= ~invalMask; }

private:
    void addParent(Node*);
    void removeParent(Node*);
    friend class ContainerNode;

    template <typename Func>
    void forEachParent(Func&&);

    class ScopedFlag;

    union {
        Node*             fParent;
        SkTDArray<Node*>* fParentArray;
    };

    uint32_t fFlags      : 16;
    uint32_t fInvalState : 2;
};

#define MAPPED_ATTRIBUTE(attr_name, attr_type, attr_container, inval_type) \
    attr_type attr_name() const { return attr_container; }                 \
    void set_##attr_name(attr_type v) {                                     \
        if (attr_container == v) return;                                   \
        attr_container = v;                                                \
        this->invalidate(inval_type);                                      \
   }

} // namespace sksg

#endif // SkSGNode_DEFINED
