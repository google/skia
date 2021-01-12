/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/pdf/SkPDFFormXObject.h"
#include "src/pdf/SkPDFUtils.h"

SkPDFIndirectReference SkPDFMakeFormXObject(SkPDFDocument* doc,
                                            std::unique_ptr<SkStreamAsset> content,
                                            std::unique_ptr<SkPDFArray> mediaBox,
                                            std::unique_ptr<SkPDFDict> resourceDict,
                                            const SkMatrix& inverseTransform,
                                            const char* colorSpace) {
    auto dict = std::make_unique<SkPDFDict>();
    dict->insert("Type", SkPDFName("XObject"));
    dict->insert("Subtype", SkPDFName("Form"));
    if (!inverseTransform.isIdentity()) {
        dict->insert("Matrix", SkPDFUtils::MatrixToArray(inverseTransform));
    }
    dict->insert("Resources", std::move(resourceDict));
    dict->insert("BBox", std::move(mediaBox));

    // Right now FormXObject is only used for saveLayer, which implies
    // isolated blending.  Do this conditionally if that changes.
    // TODO(halcanary): Is this comment obsolete, since we use it for
    // alpha masks?
    auto group = std::make_unique<SkPDFDict>();
    group->insert("Type", SkPDFName("Group"));
    group->insert("S", SkPDFName("Transparency"));
    if (colorSpace != nullptr) {
        group->insert("CS", SkPDFName(colorSpace));
    }
    group->insert("I", true);  // Isolated.
    dict->insert("Group", std::move(group));
    return SkPDFStreamOut(std::move(dict), std::move(content), doc);
}
