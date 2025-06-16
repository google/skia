/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/utils/SkClipStackUtils.h"

#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/pathops/SkPathOps.h"
#include "src/core/SkClipStack.h"

enum class SkClipOp;

void SkClipStack_AsPath(const SkClipStack& cs, SkPath* path) {
    path->reset();
    path->setFillType(SkPathFillType::kInverseEvenOdd);

    SkClipStack::Iter iter(cs, SkClipStack::Iter::kBottom_IterStart);
    while (const SkClipStack::Element* element = iter.next()) {
        if (element->getDeviceSpaceType() == SkClipStack::Element::DeviceSpaceType::kShader) {
            // TODO: Handle DeviceSpaceType::kShader somehow; it can't be turned into an SkPath
            // but perhaps the pdf backend can apply shaders in another way.
            continue;
        }
        SkPath operand;
        if (element->getDeviceSpaceType() != SkClipStack::Element::DeviceSpaceType::kEmpty) {
            element->asDeviceSpacePath(&operand);
        }

        SkClipOp elementOp = element->getOp();
        if (element->isReplaceOp()) {
            *path = operand;
            // TODO: Once expanding clip ops are removed, we can switch the iterator to be top
            // to bottom, which allows us to break here on encountering a replace op.
        } else {
            Op(*path, operand, (SkPathOp)elementOp, path);
        }
    }
}
