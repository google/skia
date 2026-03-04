/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFFormXObject.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkStream.h"
#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFUtils.h"

#include <utility>

SkPDFIndirectReference SkPDFMakeFormXObject(SkPDFDocument* doc,
                                            std::unique_ptr<SkStreamAsset> content,
                                            SkPDFParentTreeKey structParentsKey,
                                            std::unique_ptr<SkPDFArray> mediaBox,
                                            std::unique_ptr<SkPDFDict> resourceDict,
                                            const SkMatrix& inverseTransform,
                                            const char* colorSpace) {
    std::unique_ptr<SkPDFDict> dict = SkPDFMakeDict();
    dict->insertName("Type", "XObject");
    dict->insertName("Subtype", "Form");
    if (!inverseTransform.isIdentity()) {
        dict->insertObject("Matrix", SkPDFUtils::MatrixToArray(inverseTransform));
    }
    dict->insertObject("Resources", std::move(resourceDict));
    dict->insertObject("BBox", std::move(mediaBox));

    if (structParentsKey) {
        dict->insertInt("StructParents", structParentsKey.fValue);
    }
    // "StructParent" is not supported in favor of using `Do` in a marked-content sequence.

    // Right now FormXObject is only used for saveLayer, which implies
    // isolated blending.  Do this conditionally if that changes.
    // TODO(halcanary): Is this comment obsolete, since we use it for
    // alpha masks?
    auto group = SkPDFMakeDict("Group");
    group->insertName("S", "Transparency");
    if (colorSpace != nullptr) {
        group->insertName("CS", colorSpace);
    }
    group->insertBool("I", true);  // Isolated.
    dict->insertObject("Group", std::move(group));
    SkPDFIndirectReference xobject = SkPDFStreamOut(std::move(dict), std::move(content), doc);
    if (structParentsKey) {
        doc->setContentStreamRefForStructParentsKey(structParentsKey, xobject);
    }
    return xobject;
}
