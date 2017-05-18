/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPDFFormXObject.h"
#include "SkPDFUtils.h"

sk_sp<SkPDFObject> SkPDFMakeFormXObject(std::unique_ptr<SkStreamAsset> content,
                                        sk_sp<SkPDFArray> mediaBox,
                                        sk_sp<SkPDFDict> resourceDict,
                                        const SkMatrix& inverseTransform,
                                        const char* colorSpace) {
    auto form = sk_make_sp<SkPDFStream>(std::move(content));
    form->dict()->insertName("Type", "XObject");
    form->dict()->insertName("Subtype", "Form");
    if (!inverseTransform.isIdentity()) {
        sk_sp<SkPDFObject> mat(SkPDFUtils::MatrixToArray(inverseTransform));
        form->dict()->insertObject("Matrix", std::move(mat));
    }
    form->dict()->insertObject("Resources", std::move(resourceDict));
    form->dict()->insertObject("BBox", std::move(mediaBox));

    // Right now FormXObject is only used for saveLayer, which implies
    // isolated blending.  Do this conditionally if that changes.
    // TODO(halcanary): Is this comment obsolete, since we use it for
    // alpha masks?
    auto group = sk_make_sp<SkPDFDict>("Group");
    group->insertName("S", "Transparency");
    if (colorSpace != nullptr) {
        group->insertName("CS", colorSpace);
    }
    group->insertBool("I", true);  // Isolated.
    form->dict()->insertObject("Group", std::move(group));
    return form;
}
