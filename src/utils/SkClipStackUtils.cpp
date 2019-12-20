/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/pathops/SkPathOps.h"
#include "src/core/SkClipStack.h"

void SkClipStack_AsPath(const SkClipStack& cs, SkPath* path) {
    path->reset();
    path->setFillType(SkPathFillType::kInverseEvenOdd);

    SkClipStack::Iter iter(cs, SkClipStack::Iter::kBottom_IterStart);
    while (const SkClipStack::Element* element = iter.next()) {
        SkPath operand;
        if (element->getDeviceSpaceType() != SkClipStack::Element::DeviceSpaceType::kEmpty) {
            element->asDeviceSpacePath(&operand);
        }

        SkClipOp elementOp = element->getOp();
        if (elementOp == kReplace_SkClipOp) {
            *path = operand;
        } else {
            Op(*path, operand, (SkPathOp)elementOp, path);
        }
    }
}
