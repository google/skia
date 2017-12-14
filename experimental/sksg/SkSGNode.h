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

namespace sksg {

class Node : public SkRefCnt {
public:
    ~Node() override;

protected:
    Node();

    void addRef(Node*);
    void removeRef(Node*);

private:
    union {
        Node*             fRef;
        SkTDArray<Node*>* fRefArray;
    };
    mutable uint32_t      fFlags;
};

} // namespace sksg

#endif // SkSGNode_DEFINED
